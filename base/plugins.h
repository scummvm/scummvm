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

#include "common/array.h"
#include "common/error.h"
#include "common/list.h"
#include "common/singleton.h"
#include "common/util.h"
#include "base/game.h"

class Engine;
class FSList;
class MetaEngine;
class OSystem;

/**
 * Abstract base class for the plugin system.
 * Subclasses for this can be used to wrap both static and dynamic
 * plugins.
 */
class Plugin {
public:
	virtual ~Plugin() {}

	virtual bool loadPlugin() = 0;
	virtual void unloadPlugin() = 0;

	virtual const char *getName() const = 0;
	virtual const char *getCopyright() const = 0;
//	virtual int getVersion() const	{ return 0; }	// TODO!

	virtual GameList getSupportedGames() const = 0;
	virtual GameDescriptor findGame(const char *gameid) const = 0;
	virtual GameList detectGames(const FSList &fslist) const = 0;

	virtual PluginError createInstance(OSystem *syst, Engine **engine) const = 0;
};


/**
 * REGISTER_PLUGIN is a convenience macro meant to ease writing
 * the plugin interface for our modules. In particular, using it
 * makes it possible to compile the very same code in a module
 * both as a static and a dynamic plugin.
 *
 * @todo	add some means to query the plugin API version etc.
 */

#ifndef DYNAMIC_MODULES
#define REGISTER_PLUGIN(ID,METAENGINE) \
	MetaEngine *g_##ID##_MetaEngine_alloc() { \
		return new METAENGINE(); \
	} \
	void dummyFuncToAllowTrailingSemicolon()
#else
#define REGISTER_PLUGIN(ID,METAENGINE) \
	extern "C" { \
		PLUGIN_EXPORT MetaEngine *PLUGIN_MetaEngine_alloc() { \
			return new METAENGINE(); \
		} \
	} \
	void dummyFuncToAllowTrailingSemicolon()
#endif


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

/**
 * Instances of this class manage all plugins, including loading them,
 * making wrapper objects of class Plugin available, and unloading them.
 */
class PluginManager : public Common::Singleton<PluginManager> {
	typedef Common::List<PluginProvider *> ProviderList;
private:
	PluginList _plugins;
	ProviderList _providers;

	bool tryLoadPlugin(Plugin *plugin);

	friend class Common::Singleton<SingletonBaseType>;
	PluginManager();

public:
	~PluginManager();

	void addPluginProvider(PluginProvider *pp);

	void loadPlugins();
	void unloadPlugins();
	void unloadPluginsExcept(const Plugin *plugin);

	const PluginList &getPlugins()	{ return _plugins; }

	GameList detectGames(const FSList &fslist) const;
};

#endif
