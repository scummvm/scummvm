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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef ENGINES_METAENGINE_H
#define ENGINES_METAENGINE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/error.h"

#include "base/game.h"
#include "base/plugins.h"

class Engine;
class OSystem;

namespace Common {
	class FSList;
}

/**
 * A meta engine is essentially a factory for Engine instances with the
 * added ability of listing and detecting supported games.
 * Every engine "plugin" provides a hook to get an instance of a MetaEngine
 * subclass for that "engine plugin". E.g. SCUMM povides ScummMetaEngine.
 * This is then in turn used by the frontend code to detect games,
 * and instantiate actual Engine objects.
 */
class MetaEngine : public PluginObject {
public:
	virtual ~MetaEngine() {}

	/** Returns some copyright information about the engine. */
	virtual const char *getCopyright() const = 0;

	/** Returns a list of games supported by this engine. */
	virtual GameList getSupportedGames() const = 0;

	/** Query the engine for a GameDescriptor for the specified gameid, if any. */
	virtual GameDescriptor findGame(const char *gameid) const = 0;

	/**
	 * Runs the engine's game detector on the given list of files, and returns a
	 * (possibly empty) list of games supported by the engine which it was able
	 * to detect amongst the given files.
	 */
	virtual GameList detectGames(const Common::FSList &fslist) const = 0;

	/**
	 * Tries to instantiate an engine instance based on the settings of
	 * the currently active ConfMan target. That is, the MetaEngine should
	 * query the ConfMan singleton for the target, gameid, path etc. data.
	 *
	 * @param syst	Pointer to the global OSystem object
	 * @param engine	Pointer to a pointer which the MetaEngine sets to
	 *					the newly create Engine, or 0 in case of an error
	 * @return		a PluginError describing the error which occurred, or kNoError
	 */
	virtual PluginError createInstance(OSystem *syst, Engine **engine) const = 0;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * The caller has to ensure that this (Meta)Engine is responsible
	 * for the specified target (by using findGame on it respectively
	 * on the associated gameid from the relevant ConfMan entry, if present).
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target	name of a config manager target
	 * @return			a list of save state descriptors
	 */
	virtual SaveStateList listSaves(const char *target) const {
		return SaveStateList();
	}

	/**
	 * Remove the specified save state. 
	 *
	 * For most engines this just amounts to calling _saveFileMan->removeSaveFile().  
	 * Engines which keep an index file will also update it accordingly.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state to be removed
	 */
	virtual void removeSaveState(const char *target, int slot) const {};

	/**
	 * Loads a thumbnail from the specified save state.
	 *
	 * This can return '0' to indicate that no thumbnail was found.
	 * If it returns a valid Graphics::Surface object, it must be the same
	 * format as the overlay.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state
	 */
	virtual Graphics::Surface *loadThumbnailFromSlot(const char *target, int slot) const { return 0; }

	
	/** @name MetaEngineFeature flags */
	//@{
	
	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 */
	enum MetaEngineFeature {
		/** 'Return to launcher' feature (i.e. EVENT_RTL is handled) */
		kSupportsRTL            = 0,

		/**
		 * Listing Save States (i.e. implements the listSaves() method;
		 * used for --list-saves support)
		 */
		kSupportsListSaves      = 1,
		
		/** Loading from the Launcher / command line (-x) */
		kSupportsDirectLoad     = 2,

		/**
		 * Deleting Saves from the Launcher (i.e. implements the
		 * removeSaveState() method)
		 */
		kSupportsDeleteSave     = 3,

		/**
		 * Features a thumbnail in savegames (i.e. implements the
		 * loadThumbnailFromSlot method)
		 */
		kSupportsThumbnails		= 4
	};	

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 * Used by e.g. the launcher to determine whether to enable the "Load" button.
	 */	
	virtual bool hasFeature(MetaEngineFeature f) const { return false; };

	//@}
};


// Engine plugins

typedef PluginSubclass<MetaEngine> EnginePlugin;

/**
 * Singleton class which manages all Engine plugins.
 */
class EngineManager : public Common::Singleton<EngineManager> {
private:
	friend class Common::Singleton<SingletonBaseType>;

public:
	GameDescriptor findGame(const Common::String &gameName, const EnginePlugin **plugin = NULL) const;
	GameList detectGames(const Common::FSList &fslist) const;
	const EnginePlugin::List &getPlugins() const;
};

/** Convenience shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()

#endif
