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
#include "backends/fs/fs.h"
#include "base/plugins.h"
#include "base/engine.h"
#include "common/util.h"

/** Type of factory functions which make new Engine objects. */
typedef PluginError (*EngineFactory)(OSystem *syst, Engine **engine);

typedef const char *(*NameFunc)();
typedef GameDescriptor (*GameIDQueryFunc)(const char *gameid);
typedef GameList (*GameIDListFunc)();
typedef DetectedGameList (*DetectFunc)(const FSList &fslist);


#ifdef DYNAMIC_MODULES

#ifdef UNIX
#include <dlfcn.h>
#define PLUGIN_DIRECTORY	"plugins/"
#else
#ifdef __DC__
#include "dcloader.h"
#define PLUGIN_DIRECTORY	"/"
#define PLUGIN_PREFIX		""
#define PLUGIN_SUFFIX		".PLG"
#else
#ifdef _WIN32
#define PLUGIN_DIRECTORY	""
#define PLUGIN_PREFIX		""
#define PLUGIN_SUFFIX		".dll"
#else
#error No support for loading plugins on non-unix systems at this point!
#endif
#endif
#endif

#else

PluginRegistrator::PluginRegistrator(const char *name, GameList games, GameIDQueryFunc qf, EngineFactory ef, DetectFunc df)
	: _name(name), _qf(qf), _ef(ef), _df(df), _games(games) {
	//printf("Automatically registered plugin '%s'\n", name);
}

#endif


#pragma mark -


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

	const char *getName() const { return _plugin->_name; }

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
#endif

#pragma mark -

#ifdef DYNAMIC_MODULES

class DynamicPlugin : public Plugin {
	void *_dlHandle;
	Common::String _filename;

	Common::String _name;
	GameIDQueryFunc _qf;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;

	void *findSymbol(const char *symbol);

public:
	DynamicPlugin(const Common::String &filename)
		: _dlHandle(0), _filename(filename), _qf(0), _ef(0), _df(0), _games() {}

	const char *getName() const					{ return _name.c_str(); }

	PluginError createInstance(OSystem *syst, Engine **engine) const {
		assert(_ef);
		return (*_ef)(syst, engine);
	}

	GameList getSupportedGames() const { return _games; }

	GameDescriptor findGame(const char *gameid) const {
		assert(_qf);
		return (*_qf)(gameid);
	}

	DetectedGameList detectGames(const FSList &fslist) const {
		assert(_df);
		return (*_df)(fslist);
	}

	bool loadPlugin();
	void unloadPlugin();
};

void *DynamicPlugin::findSymbol(const char *symbol) {
#if defined(UNIX) || defined(__DC__)
	void *func = dlsym(_dlHandle, symbol);
	if (!func)
		warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());
	return func;
#else
#if defined(_WIN32)
	void *func = (void*)GetProcAddress((HMODULE)_dlHandle, symbol);
	if (!func)
		warning("Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());
	return func;
#else
#error TODO
#endif
#endif
}

bool DynamicPlugin::loadPlugin() {
	assert(!_dlHandle);
#if defined(UNIX) || defined(__DC__)
	_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);

	if (!_dlHandle) {
		warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
		return false;
	}
#else
#if defined(_WIN32)
	_dlHandle = LoadLibrary(_filename.c_str());

	if (!_dlHandle) {
		warning("Failed loading plugin '%s'", _filename.c_str());
		return false;
	}
#else
#error TODO
#endif
#endif

	// Query the plugin's name
	NameFunc nameFunc = (NameFunc)findSymbol("PLUGIN_name");
	if (!nameFunc) {
		unloadPlugin();
		return false;
	}
	_name = nameFunc();

	// Query the plugin for the game ids it supports
	GameIDListFunc gameListFunc = (GameIDListFunc)findSymbol("PLUGIN_gameIDList");
	if (!gameListFunc) {
		unloadPlugin();
		return false;
	}
	_games = gameListFunc();

	// Retrieve the gameid query function
	_qf = (GameIDQueryFunc)findSymbol("PLUGIN_findGameID");
	if (!_qf) {
		unloadPlugin();
		return false;
	}

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

#ifdef __DC__
	dlforgetsyms(_dlHandle);
#endif

	return true;
}

void DynamicPlugin::unloadPlugin() {
	if (_dlHandle) {
#if defined(UNIX) || defined(__DC__)
		if (dlclose(_dlHandle) != 0)
			warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
#elif defined(_WIN32)
		if (!FreeLibrary((HMODULE)_dlHandle))
			warning("Failed unloading plugin '%s'", _filename.c_str());
#else
#error TODO
#endif
	}
}

#endif	// DYNAMIC_MODULES

#pragma mark -

DECLARE_SINGLETON(PluginManager);

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

	// Load all plugins.
	// Scan for all plugins in this directory
	FilesystemNode dir(PLUGIN_DIRECTORY);
	FSList files;
	if (!dir.listDir(files, FilesystemNode::kListFilesOnly)) {
		error("Couldn't open plugin directory '%s'", PLUGIN_DIRECTORY);
	}

	for (FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
		Common::String name(i->displayName());
		if (name.hasPrefix(PLUGIN_PREFIX) && name.hasSuffix(PLUGIN_SUFFIX)) {
			tryLoadPlugin(new DynamicPlugin(i->path()));
		}
	}

#else

	#define LINK_PLUGIN(ID) \
		extern PluginRegistrator *g_##ID##_PluginReg; \
		extern void g_##ID##_PluginReg_alloc(); \
		g_##ID##_PluginReg_alloc(); \
		plugin = g_##ID##_PluginReg; \
		tryLoadPlugin(new StaticPlugin(plugin));

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
	#ifndef DISABLE_SIMON
	LINK_PLUGIN(SIMON)
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

#endif
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
