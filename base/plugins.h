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

#ifndef COMMON_PLUGINS_H
#define COMMON_PLUGINS_H

#include "common/list.h"
#include "common/singleton.h"

class Engine;
class FSList;
class GameDetector;
class OSystem;
struct GameSettings;

/** List of GameSettings- */
typedef Common::List<GameSettings> GameList;

/**
 * Abstract base class for the plugin system.
 * Subclasses for this can be used to wrap both static and dynamic
 * plugins.
 */
class Plugin {
public:
	virtual ~Plugin()				{}

	virtual bool loadPlugin()		{ return true; }
	virtual void unloadPlugin()		{}

	virtual const char *getName() const = 0;
	virtual int getVersion() const	{ return 0; }	// TODO!
	
	virtual GameList getSupportedGames() const = 0;
	virtual GameSettings findGame(const char *gameName) const;
	virtual GameList detectGames(const FSList &fslist) const = 0;

	virtual Engine *createInstance(GameDetector *detector, OSystem *syst) const = 0;
};


/**
 * The REGISTER_PLUGIN is a convenience macro meant to ease writing
 * the plugin interface for our modules. In particular, using it
 * makes it possible to compile the very same code in a module
 * both as a static and a dynamic plugin.
 *
 * @todo	add some means to query the plugin API version etc.
 * @todo	on Windows, we might need __declspec(dllexport) ?
 */
#ifndef DYNAMIC_MODULES
#define REGISTER_PLUGIN(name,gameListFactory,engineFactory,detectGames)
#else
#define REGISTER_PLUGIN(name,gameListFactory,engineFactory,detectGames) \
	extern "C" { \
		const char *PLUGIN_name() { return name; } \
		GameList PLUGIN_getSupportedGames() { return gameListFactory(); } \
		Engine *PLUGIN_createEngine(GameDetector *detector, OSystem *syst) { return engineFactory(detector, syst); } \
		GameList PLUGIN_detectGames(const FSList &fslist) { return detectGames(fslist); } \
	}
#endif


/** List of plugins. */
typedef Common::List<Plugin *> PluginList;


/**
 * Instances of this class manage all plugins, including loading them,
 * making wrapper objects of class Plugin available, and unloading them.
 *
 * @todo Add support for dynamic plugins (this may need additional API, e.g. for a plugin path)
 */
class PluginManager : public Common::Singleton<PluginManager> {
private:
	PluginList _plugins;
	
	bool tryLoadPlugin(Plugin *plugin);
	
	friend class Common::Singleton<PluginManager>;
	PluginManager();

public:
	~PluginManager();

	void loadPlugins();
	void unloadPlugins();
	
	const PluginList &getPlugins()	{ return _plugins; }
};


#ifndef DYNAMIC_MODULES

#define DECLARE_PLUGIN(name) \
	extern GameList Engine_##name##_gameList(); \
	extern Engine *Engine_##name##_create(GameDetector *detector, OSystem *syst); \
	extern GameList Engine_##name##_detectGames(const FSList &fslist);

// Factory functions => no need to include the specific classes
// in this header. This serves two purposes:
// 1) Clean separation from the game modules (scumm, simon) and the generic code
// 2) Faster (compiler doesn't have to parse lengthy header files)
#ifndef DISABLE_SCUMM
DECLARE_PLUGIN(SCUMM)
#endif

#ifndef DISABLE_SIMON
DECLARE_PLUGIN(SIMON)
#endif

#ifndef DISABLE_SKY
DECLARE_PLUGIN(SKY)
#endif

#ifndef DISABLE_SWORD2
DECLARE_PLUGIN(SWORD2)
#endif

#ifndef DISABLE_QUEEN
DECLARE_PLUGIN(QUEEN)
#endif

#endif

#endif
