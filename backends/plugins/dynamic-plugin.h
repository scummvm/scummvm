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

#ifndef BACKENDS_PLUGINS_DYNAMICPLUGIN_H
#define BACKENDS_PLUGINS_DYNAMICPLUGIN_H

#include "base/plugins.h"
#include "engines/metaengine.h"


class DynamicPlugin : public Plugin {
protected:
	typedef void (*VoidFunc)();

	typedef MetaEngine *(*MetaAllocFunc)();

	MetaEngine *_metaengine;

	virtual VoidFunc findSymbol(const char *symbol) = 0;

public:
	DynamicPlugin() : _metaengine(0) {}

	const char *getName() const {
		return _metaengine->getName();
	}

	const char *getCopyright() const {
		return _metaengine->getCopyright();
	}

	PluginError createInstance(OSystem *syst, Engine **engine) const {
		return _metaengine->createInstance(syst, engine);
	}

	GameList getSupportedGames() const {
		return _metaengine->getSupportedGames();
	}

	GameDescriptor findGame(const char *gameid) const {
		return _metaengine->findGame(gameid);
	}

	GameList detectGames(const FSList &fslist) const {
		return _metaengine->detectGames(fslist);
	}

	virtual bool loadPlugin() {
		// Query the plugin's name
		MetaAllocFunc metaAlloc = (MetaAllocFunc)findSymbol("PLUGIN_MetaEngine_alloc");
		if (!metaAlloc) {
			unloadPlugin();
			return false;
		}

		_metaengine = metaAlloc();
		if (!_metaengine) {
			unloadPlugin();
			return false;
		}

		return true;
	}
	
	virtual void unloadPlugin() {
		delete _metaengine;
	}
};

#endif
