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

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/engine.h"
#include "common/util.h"


typedef Engine *(*EngineFactory)(GameDetector *detector, OSystem *syst);


#ifdef DYNAMIC_MODULES

#ifdef UNIX
#include <dlfcn.h>
#else
#error No support for loading plugins on non-unix systems at this point!
#endif

#else

// Factory functions => no need to include the specific classes
// in this header. This serves two purposes:
// 1) Clean seperation from the game modules (scumm, simon) and the generic code
// 2) Faster (compiler doesn't have to parse lengthy header files)
#ifndef DISABLE_SCUMM
extern const GameSettings *Engine_SCUMM_targetList();
extern Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SIMON
extern Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst);
extern const GameSettings *Engine_SIMON_targetList();
#endif

#ifndef DISABLE_SKY
extern const GameSettings *Engine_SKY_targetList();
extern Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SWORD2
extern const GameSettings *Engine_SWORD2_targetList();
extern Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_QUEEN
extern const GameSettings *Engine_QUEEN_targetList();
extern Engine *Engine_QUEEN_create(GameDetector *detector, OSystem *syst);
#endif

#endif


#pragma mark -


int Plugin::countSupportedGames() const {
	const GameSettings *target = getSupportedGames();
	int count;
	for (count = 0; target->gameName; target++, count++)
		;
	return count;
}

const GameSettings *Plugin::findGame(const char *gameName) const {
	// Find the GameSettings for this target
	const GameSettings *target = getSupportedGames();
	assert(gameName);
	while (target->gameName) {
		if (!scumm_stricmp(target->gameName, gameName)) {
			return target;
		}
		target++;
	}
	return 0;
}


#pragma mark -


class StaticPlugin : public Plugin {
	const char *_name;
	const GameSettings *_targets;
	int _targetCount;
	EngineFactory _ef;
public:
	StaticPlugin(const char *name, const GameSettings *targets, EngineFactory ef)
		: _name(name), _targets(targets), _ef(ef) {
		_targetCount = Plugin::countSupportedGames();
	}

	const char *getName() const					{ return _name; }

	int countSupportedGames() const					{ return _targetCount; }
	const GameSettings *getSupportedGames() const	{ return _targets; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		return (*_ef)(detector, syst);
	}
};


#pragma mark -


#ifdef DYNAMIC_MODULES

class DynamicPlugin : public Plugin {
	void *_dlHandle;
	Common::String _filename;

	Common::String _name;
	const GameSettings *_targets;
	int _targetCount;
	EngineFactory _ef;
	
	void *findSymbol(const char *symbol);

public:
	DynamicPlugin(const char *filename)
		: _dlHandle(0), _filename(filename), _targets(0), _targetCount(0), _ef(0) {}
	
	const char *getName() const					{ return _name.c_str(); }

	int countSupportedGames() const					{ return _targetCount; }
	const GameSettings *getSupportedGames() const	{ return _targets; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		assert(_ef);
		return (*_ef)(detector, syst);
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

typedef const char *(*NameFunc)();
typedef const GameSettings *(*TargetListFunc)();

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
	TargetListFunc targetListFunc = (TargetListFunc)findSymbol("PLUGIN_getTargetList");
	if (!targetListFunc) {
		unloadPlugin();
		return false;
	}
	_targets = targetListFunc();
	
	// Finally, retrieve the factory function
	_ef = (EngineFactory)findSymbol("PLUGIN_createEngine");
	if (!_ef) {
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
#ifndef DYNAMIC_MODULES
	// "Load" the static plugins
	#ifndef DISABLE_SCUMM
		tryLoadPlugin(new StaticPlugin("scumm", Engine_SCUMM_targetList(), Engine_SCUMM_create));
	#endif
	
	#ifndef DISABLE_SIMON
		tryLoadPlugin(new StaticPlugin("simon", Engine_SIMON_targetList(), Engine_SIMON_create));
	#endif
	
	#ifndef DISABLE_SKY
		tryLoadPlugin(new StaticPlugin("sky", Engine_SKY_targetList(), Engine_SKY_create));
	#endif
	
	#ifndef DISABLE_SWORD2
		tryLoadPlugin(new StaticPlugin("sword2", Engine_SWORD2_targetList(), Engine_SWORD2_create));
	#endif
	
	#ifndef DISABLE_QUEEN
		tryLoadPlugin(new StaticPlugin("queen", Engine_QUEEN_targetList(), Engine_QUEEN_create));
	#endif
		
#else
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
	#ifndef DISABLE_SCUMM
		tryLoadPlugin(new DynamicPlugin("scumm/libscumm.so"));
	#endif
	
	#ifndef DISABLE_SIMON
		tryLoadPlugin(new DynamicPlugin("simon/libsimon.so"));
	#endif
	
	#ifndef DISABLE_SKY
		tryLoadPlugin(new DynamicPlugin("sky/libsky.so"));
	#endif
	
	#ifndef DISABLE_SWORD2
		tryLoadPlugin(new DynamicPlugin("bs2/libbs2.so"));
	#endif

	#ifndef DISABLE_QUEEN
		tryLoadPlugin(new DynamicPlugin("queen/libqueen.so"));
	#endif
		
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

