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
extern const TargetSettings *Engine_SCUMM_targetList();
extern Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SIMON
extern Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst);
extern const TargetSettings *Engine_SIMON_targetList();
#endif

#ifndef DISABLE_SKY
extern const TargetSettings *Engine_SKY_targetList();
extern Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SWORD2
extern const TargetSettings *Engine_SWORD2_targetList();
extern Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst);
#endif

#endif


#pragma mark -


PluginManager	*g_pluginManager = 0;


#pragma mark -


int Plugin::countTargets() const {
	const TargetSettings *target = getTargets();
	int count;
	for (count = 0; target->targetName; target++, count++)
		;
	return count;
}

const TargetSettings *Plugin::findTarget(const char *targetName) const {
	// Find the TargetSettings for this target
	const TargetSettings *target = getTargets();
	assert(targetName);
	while (target->targetName) {
		if (!scumm_stricmp(target->targetName, targetName)) {
			return target;
		}
		target++;
	}
	return 0;
}


#pragma mark -


class StaticPlugin : public Plugin {
	const char *_name;
	const TargetSettings *_targets;
	int _targetCount;
	EngineFactory _ef;
public:
	StaticPlugin(const char *name, const TargetSettings *targets, EngineFactory ef)
		: _name(name), _targets(targets), _ef(ef) {
		_targetCount = Plugin::countTargets();
	}

	const char *getName() const					{ return _name; }

	int countTargets() const					{ return _targetCount; }
	const TargetSettings *getTargets() const	{ return _targets; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		return (*_ef)(detector, syst);
	}
};


#pragma mark -


#ifdef DYNAMIC_MODULES

class DynamicPlugin : public Plugin {
	void *_dlHandle;
	ScummVM::String _filename;

	ScummVM::String _name;
	const TargetSettings *_targets;
	int _targetCount;
	EngineFactory _ef;
	
	void *findSymbol(const char *symbol);

public:
	DynamicPlugin(const char *filename)
		: _dlHandle(0), _filename(filename), _targets(0), _targetCount(0), _ef(0) {}
	
	const char *getName() const					{ return _name.c_str(); }

	int countTargets() const					{ return _targetCount; }
	const TargetSettings *getTargets() const	{ return _targets; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		assert(_ef);
		return (*_ef)(detector, syst);
	}

	bool loadPlugin();
	void unloadPlugin();
};

void *DynamicPlugin::findSymbol(const char *symbol) {
#ifdef UNIX
	void *func = dlsym(_dlHandle, symbol);
	if (!func)
		warning("Failed loading symbold '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());
	return func;
#else
#error TODO
#endif
}

typedef const char *(*NameFunc)();
typedef const TargetSettings *(*TargetListFunc)();

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
#else
	// Load dynamic plugins
	// TODO... this is right now just a nasty hack. 
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

