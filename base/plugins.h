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
 * $Header$
 *
 */

#ifndef COMMON_PLUGINS_H
#define COMMON_PLUGINS_H

#include "common/array.h"
#include "common/singleton.h"
#include "common/util.h"

class Engine;
class FSList;
class GameDetector;
class OSystem;
struct GameSettings;

/** List of games. */
typedef Common::Array<GameSettings> GameList;

/**
 * A detected game. Carries the GameSettings, but also (optionally)
 * information about the language and platform of the detected game.
 */
struct DetectedGame : GameSettings {
	Common::Language language;
	Common::Platform platform;
	DetectedGame() : language(Common::UNK_LANG), platform(Common::kPlatformUnknown) {}
	DetectedGame(const GameSettings &game,
	             Common::Language l = Common::UNK_LANG,
	             Common::Platform p = Common::kPlatformUnknown)
		: GameSettings(game), language(l), platform(p) {}
};

/** List of detected games. */
typedef Common::Array<DetectedGame> DetectedGameList;


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
	virtual DetectedGameList detectGames(const FSList &fslist) const = 0;

	virtual Engine *createInstance(GameDetector *detector, OSystem *syst) const = 0;
};


/**
 * REGISTER_PLUGIN is a convenience macro meant to ease writing
 * the plugin interface for our modules. In particular, using it
 * makes it possible to compile the very same code in a module
 * both as a static and a dynamic plugin.
 *
 * @todo	add some means to query the plugin API version etc.
 * @todo	on Windows, we might need __declspec(dllexport) ?
 */

#if defined(PALMOS_ARM) || defined(PALMOS_DEBUG)
#define REGISTER_PLUGIN(ID,name) \
	PluginRegistrator *g_##ID##_PluginReg; \
	void g_##ID##_PluginReg_alloc() { \
		g_##ID##_PluginReg = new PluginRegistrator(name, Engine_##ID##_gameList(), Engine_##ID##_create, Engine_##ID##_detectGames);\
	}
#else

#ifndef DYNAMIC_MODULES
#define REGISTER_PLUGIN(ID,name) \
	PluginRegistrator g_##ID##_PluginReg(name, Engine_##ID##_gameList(), Engine_##ID##_create, Engine_##ID##_detectGames);
#else
#define REGISTER_PLUGIN(ID,name) \
	extern "C" { \
		PLUGIN_EXPORT const char *PLUGIN_name() { return name; } \
		PLUGIN_EXPORT GameList PLUGIN_getSupportedGames() { return Engine_##ID##_gameList(); } \
		PLUGIN_EXPORT Engine *PLUGIN_createEngine(GameDetector *detector, OSystem *syst) { return Engine_##ID##_create(detector, syst); } \
		PLUGIN_EXPORT DetectedGameList PLUGIN_detectGames(const FSList &fslist) { return Engine_##ID##_detectGames(fslist); } \
	}
#endif
#endif

#ifndef DYNAMIC_MODULES
/**
 * The PluginRegistrator class is used by the static version of REGISTER_PLUGIN
 * to allow static 'plugins' to register with the PluginManager.
 */
class PluginRegistrator {
	friend class PluginManager;
public:
	typedef Engine *(*EngineFactory)(GameDetector *detector, OSystem *syst);
	typedef DetectedGameList (*DetectFunc)(const FSList &fslist);

protected:
	const char *_name;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;

public:
	PluginRegistrator(const char *name, GameList games, EngineFactory ef, DetectFunc df);
};
#endif


/** List of plugins. */
typedef Common::Array<Plugin *> PluginList;


class PluginManager;

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

	friend class Common::Singleton<SingletonBaseType>;
	PluginManager();

public:
	~PluginManager();

	void loadPlugins();
	void unloadPlugins();
	void unloadPluginsExcept(const Plugin *plugin);

	const PluginList &getPlugins()	{ return _plugins; }

	DetectedGameList detectGames(const FSList &fslist) const;
};

#endif
