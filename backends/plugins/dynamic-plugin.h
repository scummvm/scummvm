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

#ifndef BACKENDS_PLUGINS_DYNAMICPLUGIN_H
#define BACKENDS_PLUGINS_DYNAMICPLUGIN_H

#include "common/stdafx.h"
#include "base/plugins.h"


/** Type of factory functions which make new Engine objects. */
typedef PluginError (*EngineFactory)(OSystem *syst, Engine **engine);

typedef const char *(*NameFunc)();
typedef GameDescriptor (*GameIDQueryFunc)(const char *gameid);
typedef GameList (*GameIDListFunc)();
typedef DetectedGameList (*DetectFunc)(const FSList &fslist);


class DynamicPlugin : public Plugin {
protected:
	typedef void (*VoidFunc)();
	
	Common::String _name;
	Common::String _copyright;
	GameIDQueryFunc _qf;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;

	virtual VoidFunc findSymbol(const char *symbol) = 0;

public:
	DynamicPlugin() : _qf(0), _ef(0), _df(0), _games() {}

	const char *getName() const					{ return _name.c_str(); }
	const char *getCopyright() const			{ return _copyright.c_str(); }

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

	virtual bool loadPlugin() {
		// Query the plugin's name
		NameFunc nameFunc = (NameFunc)findSymbol("PLUGIN_name");
		if (!nameFunc) {
			unloadPlugin();
			return false;
		}
		_name = nameFunc();
	
		// Query the plugin's copyright
		nameFunc = (NameFunc)findSymbol("PLUGIN_copyright");
		if (!nameFunc) {
			unloadPlugin();
			return false;
		}
		_copyright = nameFunc();
	
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
	
		return true;
	}
};

#endif
