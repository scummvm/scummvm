/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BASE_PLUGINS_H
#define BASE_PLUGINS_H

#include "common/array.h"
#include "common/fs.h"
#include "common/str.h"
#include "backends/plugins/elf/version.h"

#define INCLUDED_FROM_BASE_PLUGINS_H
#include "base/internal_plugins.h"
#undef INCLUDED_FROM_BASE_PLUGINS_H


// Plugin versioning

/** Global Plugin API version */
#define PLUGIN_VERSION 1

enum PluginType {
	PLUGIN_TYPE_ENGINE_DETECTION = 0,
	PLUGIN_TYPE_ENGINE,
	PLUGIN_TYPE_MUSIC,
	PLUGIN_TYPE_DETECTION,
	PLUGIN_TYPE_SCALER,

	PLUGIN_TYPE_MAX
};

// TODO: Make the engine API version depend on ScummVM's version
// because of the back linking (possibly from the checkout revision)
#define PLUGIN_TYPE_ENGINE_DETECTION_VERSION 1
#define PLUGIN_TYPE_ENGINE_VERSION 2
#define PLUGIN_TYPE_MUSIC_VERSION 1
#define PLUGIN_TYPE_DETECTION_VERSION 1
#define PLUGIN_TYPE_SCALER_VERSION 1

extern const int pluginTypeVersions[PLUGIN_TYPE_MAX];


// Plugin linking

// see comments in backends/plugins/elf/elf-provider.cpp
#if defined(USE_ELF_LOADER) && defined(ELF_LOADER_CXA_ATEXIT)
#define PLUGIN_DYNAMIC_DSO_HANDLE \
	uint32 __dso_handle __attribute__((visibility("hidden"))) = 0;
#else
#define PLUGIN_DYNAMIC_DSO_HANDLE
#endif

#ifdef USE_ELF_LOADER
#define PLUGIN_DYNAMIC_BUILD_DATE \
	PLUGIN_EXPORT const char *PLUGIN_getBuildDate() { return gScummVMPluginBuildDate; }
#else
#define PLUGIN_DYNAMIC_BUILD_DATE
#endif

/**
 * REGISTER_PLUGIN_STATIC is a convenience macro which is used to declare
 * the plugin interface for static plugins. Code (such as game engines)
 * which needs to implement a static plugin can simply invoke this macro
 * with a plugin ID, plugin type and PluginObject subclass, and the correct
 * wrapper code will be inserted.
 *
 * @see REGISTER_PLUGIN_DYNAMIC
 */
#define REGISTER_PLUGIN_STATIC(ID,TYPE,PLUGINCLASS) \
	extern const PluginType g_##ID##_type; \
	const PluginType g_##ID##_type = TYPE; \
	PluginObject *g_##ID##_getObject() { \
		return new PLUGINCLASS(); \
	} \
	void dummyFuncToAllowTrailingSemicolon_##ID##_()

#ifdef DYNAMIC_MODULES

/**
 * REGISTER_PLUGIN_DYNAMIC is a convenience macro which is used to declare
 * the plugin interface for dynamically loadable plugins. Code (such as game engines)
 * which needs to implement a dynamic plugin can simply invoke this macro
 * with a plugin ID, plugin type and PluginObject subclass, and the correct
 * wrapper code will be inserted.
 *
 * @see REGISTER_PLUGIN_STATIC
 */
#define REGISTER_PLUGIN_DYNAMIC(ID,TYPE,PLUGINCLASS) \
	extern "C" { \
		PLUGIN_DYNAMIC_DSO_HANDLE \
		PLUGIN_DYNAMIC_BUILD_DATE \
		PLUGIN_EXPORT int32 PLUGIN_getVersion() { return PLUGIN_VERSION; } \
		PLUGIN_EXPORT int32 PLUGIN_getType() { return TYPE; } \
		PLUGIN_EXPORT int32 PLUGIN_getTypeVersion() { return TYPE##_VERSION; } \
		PLUGIN_EXPORT PluginObject *PLUGIN_getObject() { \
			return new PLUGINCLASS(); \
		} \
	} \
	void dummyFuncToAllowTrailingSemicolon_##ID##_()

#endif // DYNAMIC_MODULES


// Abstract plugins

/**
 * Abstract base class for the plugin objects which handle plugins
 * instantiation. Subclasses for this may be used for engine plugins and other
 * types of plugins. An existing PluginObject refers to an executable file
 * loaded in memory and ready to run. The plugin, on the other hand, is just
 * a handle to the file/object, whether it's loaded in memory or not.
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
 * plugins. This class refers to a plugin which may or may not be loaded in
 * memory.
 */
class Plugin {
protected:
	PluginObject *_pluginObject;
	PluginType _type;

public:
	Plugin() : _pluginObject(0), _type(PLUGIN_TYPE_MAX) {}
	virtual ~Plugin() {
		//if (isLoaded())
			//unloadPlugin();
	}

//	virtual bool isLoaded() const = 0; // TODO
	virtual bool loadPlugin() = 0;     // TODO: Rename to load() ?
	virtual void unloadPlugin() = 0;   // TODO: Rename to unload() ?

	/**
	 * The following functions query information from the plugin object once
	 * it's loaded into memory.
	 **/
	PluginType getType() const;
	const char *getName() const;

	template <class T>
	T &get() const {
		T *pluginObject = dynamic_cast<T *>(_pluginObject);
		if (!pluginObject) {
			error("Invalid cast of plugin %s", getName());
		}
		return *pluginObject;
	}

	/**
	 * The getFileName() function gets the name of the plugin file for those
	 * plugins that have files (ie. not static). It doesn't require the plugin
	 * object to be loaded into memory, unlike getName()
	 **/
	virtual Common::Path getFileName() const { return Common::Path(); }
};

class StaticPlugin : public Plugin {
public:
	StaticPlugin(PluginObject *pluginobject, PluginType type);
	~StaticPlugin();
	virtual bool loadPlugin();
	virtual void unloadPlugin();
};


/** List of Plugin instances. */
typedef Common::Array<Plugin *> PluginList;

/**
 * Abstract base class for Plugin factories. Subclasses of this
 * are responsible for creating plugin objects, e.g. by loading
 * loadable modules from storage media; by creating "fake" plugins
 * from static code; or whatever other means.
 */
class PluginProvider {
public:
	virtual ~PluginProvider() {}

	/**
	 * Return a list of Plugin objects. The caller is responsible for actually
	 * loading/unloading them (by invoking the appropriate Plugin methods).
	 * Furthermore, the caller is responsible for deleting these objects
	 * eventually.
	 *
	 * @return a list of Plugin instances
	 */
	virtual PluginList getPlugins() = 0;

	/**
	 * @return whether or not object is a FilePluginProvider.
	 */
	virtual bool isFilePluginProvider() { return false; }
};

#ifdef DYNAMIC_MODULES

/**
 * Abstract base class for Plugin factories which load binary code from files.
 * Subclasses only have to implement the createPlugin() method, and optionally
 * can overload the other protected methods to achieve custom behavior.
 */
class FilePluginProvider : public PluginProvider {
public:
	/**
	 * Return a list of Plugin objects loaded via createPlugin from disk.
	 * For this, a list of directories is searched for plugin objects:
	 * The current dir and its "plugins" subdirectory (if present), a list
	 * of custom search dirs (see addCustomDirectories) and finally the
	 * directory specified via the "pluginspath" config variable (if any).
	 *
	 * @return a list of Plugin instances
	 */
	virtual PluginList getPlugins();

	/**
	 * @return whether or not object is a FilePluginProvider.
	 */
	bool isFilePluginProvider() { return true; }

protected:
	/**
	 * Create a Plugin instance from a loadable code module with the specified name.
	 * Subclasses of FilePluginProvider have to at least overload this method.
	 * If the file is not found, or does not contain loadable code, 0 is returned instead.
	 *
	 * @param node	the FSNode of the loadable code module
	 * @return	a pointer to a Plugin instance, or 0 if an error occurred.
	 */
	virtual Plugin *createPlugin(const Common::FSNode &node) const = 0;

	/**
	 * Check if the supplied file corresponds to a loadable plugin file in
	 * the current platform. Usually, this will just check the file name.
	 *
	 * @param node	the FSNode of the file to check
	 * @return	true if the filename corresponds to a plugin, false otherwise
	 */
	virtual bool isPluginFilename(const Common::FSNode &node) const;

	/**
	 * Optionally add to the list of directories to be searched for
	 * plugins by getPlugins().
	 *
	 * @param dirs	the reference to the list of directories to be used when
	 *		searching for plugins.
	 */
	virtual void addCustomDirectories(Common::FSList &dirs) const;
};

#endif // DYNAMIC_MODULES

#define PluginMan PluginManager::instance()

/**
 * Singleton class which manages all plugins, including loading them,
 * managing all Plugin class instances, and unloading them.
 */
class PluginManager {
protected:
	typedef Common::Array<PluginProvider *> ProviderList;

	PluginList _pluginsInMem[PLUGIN_TYPE_MAX];
	ProviderList _providers;

	bool tryLoadPlugin(Plugin *plugin);
	void addToPluginsInMemList(Plugin *plugin);
	const Plugin *findLoadedPlugin(const Common::String &engineId);

	static PluginManager *_instance;
	PluginManager();

	void unloadAllPlugins();
public:
	virtual ~PluginManager();

	static void destroy() { delete _instance; _instance = 0; }
	static PluginManager &instance();

	void addPluginProvider(PluginProvider *pp);

	/**
	 * A method which finds the METAENGINE plugin for the provided engineId
	 *
	 * @param engineId The engine ID
	 *
	 * @return A plugin of type METAENGINE.
	 */
	const Plugin *findEnginePlugin(const Common::String &engineId);

	// Functions used by the uncached PluginManager
	virtual void init()	{}
	virtual void loadFirstPlugin() {}
	virtual bool loadNextPlugin() { return false; }
	virtual bool loadPluginFromEngineId(const Common::String &engineId) { return false; }
	virtual void updateConfigWithFileName(const Common::String &engineId) {}
	virtual void loadDetectionPlugin() {}
	virtual void unloadDetectionPlugin() {}

	// Functions used only by the cached PluginManager
	virtual void loadAllPlugins();
	virtual void loadAllPluginsOfType(PluginType type);

	void unloadPluginsExcept(PluginType type, const Plugin *plugin, bool deletePlugin = true);

	const PluginList &getPlugins(PluginType t) { return _pluginsInMem[t]; }
};

/**
 *  Uncached version of plugin manager
 *  Keeps only one dynamic plugin in memory at a time
 **/
class PluginManagerUncached : public PluginManager {
protected:
	friend class PluginManager;
	PluginList _allEnginePlugins;
	Plugin  *_detectionPlugin;
	PluginList::iterator _currentPlugin;

	bool _isDetectionLoaded;

	PluginManagerUncached() : _detectionPlugin(nullptr), _currentPlugin(nullptr), _isDetectionLoaded(false) {}
	bool loadPluginByFileName(const Common::Path &filename);

public:
	virtual ~PluginManagerUncached();
	void init() override;
	void loadFirstPlugin() override;
	bool loadNextPlugin() override;
	bool loadPluginFromEngineId(const Common::String &engineId) override;
	void updateConfigWithFileName(const Common::String &engineId) override;
#ifndef DETECTION_STATIC
	void loadDetectionPlugin() override;
	void unloadDetectionPlugin() override;
#endif

	void loadAllPlugins() override {} 	// we don't allow these
	void loadAllPluginsOfType(PluginType type) override {}
};

#endif
