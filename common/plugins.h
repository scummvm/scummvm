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

class Engine;
class GameDetector;
class OSystem;
struct TargetSettings;

/**
 * Abstract base class for the plugin system.
 * Subclasses for this can be used to wrap both static and dynamic
 * plugins.
 */
class Plugin {
public:
	virtual void loadPlugin()		{}
	virtual void unloadPlugin()		{}

	virtual const char *getName() const = 0;
	virtual int getVersion() const = 0;
	
	virtual const TargetSettings *getTargets() const = 0;
	virtual Engine *createInstance(GameDetector *detector, OSystem *syst) const = 0;
};

/**
 * Instances of this class manage all plugins, including loading them,
 * making wrapper objects of class Plugin available, and unloading them.
 *
 * @todo Add support for dynamic plugins (this may need additional API, e.g. for a plugin path)
 */
class PluginManager {
protected:
	typedef ScummVM::List<Plugin *> PluginList;

	PluginList _plugins;
	
public:
	PluginManager();
	~PluginManager();
	
	void loadPlugins();
	void unloadPlugins();
	
	const PluginList &getPlugins()	{ return _plugins; }
};


#endif
