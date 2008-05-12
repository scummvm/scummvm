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

#ifndef BASE_PLUGINS_H
#define BASE_PLUGINS_H

#include "common/error.h"
#include "common/list.h"
#include "common/singleton.h"
#include "base/game.h"

// Plugin versioning

// Global Plugin API version
#define PLUGIN_VERSION 1

enum PluginType {
	PLUGIN_TYPE_ENGINE = 0,

	PLUGIN_TYPE_MAX
};

// TODO: Make the engine API version depend on ScummVM's version
// because of the backlinking
#define PLUGIN_TYPE_ENGINE_VERSION 1

extern int pluginTypeVersions[PLUGIN_TYPE_MAX];


// Plugin linking

#define STATIC_PLUGIN 1
#define DYNAMIC_PLUGIN 2

#define PLUGIN_ENABLED_STATIC(ID) \
	(defined(ENABLE_##ID) && !PLUGIN_ENABLED_DYNAMIC(ID))

// HACK for MSVC
#if defined(_MSC_VER)
	#undef PLUGIN_ENABLED_STATIC
	#define PLUGIN_ENABLED_STATIC(ID) 1
#endif

#define PLUGIN_ENABLED_DYNAMIC(ID) \
	(defined(ENABLE_##ID) && (ENABLE_##ID == DYNAMIC_PLUGIN) && defined(DYNAMIC_MODULES))

/**
 * REGISTER_PLUGIN is a convenience macro meant to ease writing
 * the plugin interface for our modules. In particular, using it
 * makes it possible to compile the very same code in a module
 * both as a static and a dynamic plugin.
 *
 * @todo	add some means to query the plugin API version etc.
 */

#define REGISTER_PLUGIN_STATIC(ID,TYPE,PLUGINCLASS) \
	PluginType g_##ID##_type = TYPE; \
	PluginObject *g_##ID##_getObject() { \
		return new PLUGINCLASS(); \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#ifdef DYNAMIC_MODULES

#define REGISTER_PLUGIN_DYNAMIC(ID,TYPE,PLUGINCLASS) \
	extern "C" { \
		PLUGIN_EXPORT int32 PLUGIN_getVersion() { return PLUGIN_VERSION; } \
		PLUGIN_EXPORT int32 PLUGIN_getType() { return TYPE; } \
		PLUGIN_EXPORT int32 PLUGIN_getTypeVersion() { return TYPE##_VERSION; } \
		PLUGIN_EXPORT PluginObject *PLUGIN_getObject() { \
			return new PLUGINCLASS(); \
		} \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#endif // DYNAMIC_MODULES


// Abstract plugins

/**
 * Abstract base class for the plugin objects which handle plugins
 * instantiation. Subclasses for this may be used for engine plugins
 * and other types of plugins.
 */
class PluginObject {
public:
	virtual ~PluginObject() {}

	/** Returns the name of the plugin. */
	virtual const char *getName() const = 0;
};

/**
 * Abstract base class for the plugin system.
 * Subclasses for this can be used to wrap both static and dynamic
 * plugins.
 */
class Plugin {
protected:
	PluginObject *_pluginObject;
	PluginType _type;

public:
	Plugin() : _pluginObject(0) {}
	virtual ~Plugin() {
		//if (isLoaded())
			//unloadPlugin();
	}

//	virtual bool isLoaded() const = 0;	// TODO
	virtual bool loadPlugin() = 0;	// TODO: Rename to load() ?
	virtual void unloadPlugin() = 0;	// TODO: Rename to unload() ?

	PluginType getType() const;
	const char *getName() const;
};

/** List of plugins. */
typedef Common::Array<Plugin *> PluginList;

class PluginProvider {
public:
	virtual ~PluginProvider() {}

	/**
	 * Return a list of Plugin objects. The caller is responsible for actually
	 * loading/unloading them (by invoking the appropriate methods).
	 * Furthermore, the caller is responsible for deleting these objects
	 * eventually.
	 */
	virtual PluginList getPlugins() = 0;
};

class FilePluginProvider : public PluginProvider {
public:
	virtual PluginList getPlugins();

protected:
	virtual Plugin* createPlugin(const Common::String &filename) const = 0;

	virtual const char* getPrefix() const;
	virtual const char* getSuffix() const;

	virtual void addCustomDirectories(Common::StringList &dirs) const;
};

/**
 * Instances of this class manage all plugins, including loading them,
 * making wrapper objects of class Plugin available, and unloading them.
 */
class PluginManager : public Common::Singleton<PluginManager> {
	typedef Common::List<PluginProvider *> ProviderList;
private:
	PluginList _plugins[PLUGIN_TYPE_MAX];
	ProviderList _providers;

	bool tryLoadPlugin(Plugin *plugin);

	friend class Common::Singleton<SingletonBaseType>;
	PluginManager();

public:
	~PluginManager();

	void addPluginProvider(PluginProvider *pp);

	void loadPlugins();
	void unloadPlugins();
	void unloadPluginsExcept(PluginType type, const Plugin *plugin);

	const PluginList &getPlugins(PluginType t) { return _plugins[t]; }
};


// Engine plugins

class Engine;
class FSList;
class OSystem;

class EnginePlugin : public Plugin {
public:
	const char *getCopyright() const;
	PluginError createInstance(OSystem *syst, Engine **engine) const;
	GameList getSupportedGames() const;
	GameDescriptor findGame(const char *gameid) const;
	GameList detectGames(const FSList &fslist) const;
	SaveStateList listSaves(const char *target) const;
};

typedef Common::Array<EnginePlugin *> EnginePluginList;

class EngineManager : public Common::Singleton<EngineManager> {
private:
	friend class Common::Singleton<SingletonBaseType>;

public:
	GameDescriptor findGame(const Common::String &gameName, const EnginePlugin **plugin = NULL) const;
	GameList detectGames(const FSList &fslist) const;
	const EnginePluginList &getPlugins() const;
};

/** Shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()

#endif
