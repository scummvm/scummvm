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
 */

#ifndef ENGINES_METAENGINE_H
#define ENGINES_METAENGINE_H

#include "common/achievements.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/array.h"

#include "engines/game.h"
#include "engines/savestate.h"

#include "base/plugins.h"

class Engine;
class OSystem;

namespace Common {
class Keymap;
class FSList;
class OutSaveFile;
class String;

typedef SeekableReadStream InSaveFile;
}

namespace Graphics {
struct Surface;
}

namespace GUI {
class GuiObject;
class OptionsContainerWidget;
}

/**
 * Per-game extra GUI options structure.
 * Currently, this can only be used for options with checkboxes.
 */
struct ExtraGuiOption {
	const char *label;          // option label, e.g. "Fullscreen mode"
	const char *tooltip;        // option tooltip (when the mouse hovers above it)
	const char *configOption;   // confMan key, e.g. "fullscreen"
	bool defaultState;          // the detault state of the checkbox (checked or not)
};

typedef Common::Array<ExtraGuiOption> ExtraGuiOptions;

#define EXTENDED_SAVE_VERSION 4

struct ExtendedSavegameHeader {
	char id[6];
	uint8 version;
	Common::String saveName;
	Common::String description;
	uint32 date;
	uint16 time;
	uint32 playtime;
	Graphics::Surface *thumbnail;
	bool isAutosave;

	ExtendedSavegameHeader() {
		memset(id, 0, 6);
		version = 0;
		date = 0;
		time = 0;
		playtime = 0;
		thumbnail = nullptr;
		isAutosave = false;
	}
};

/**
 * A meta engine is essentially a factory for Engine instances with the
 * added ability of listing and detecting supported games.
 * Every engine "plugin" provides a hook to get an instance of a MetaEngine
 * subclass for that "engine plugin". E.g. SCUMM povides ScummMetaEngine.
 * This is then in turn used by the frontend code to detect games,
 * and instantiate actual Engine objects.
 */
class MetaEngine : public PluginObject {
private:
	/**
	 * Converts the current screen contents to a thumbnail, and saves it
	 */
	static void saveScreenThumbnail(Common::OutSaveFile *saveFile);
public:
	virtual ~MetaEngine() {}

	/** Get the engine ID */
	virtual const char *getEngineId() const = 0;

	/** Returns some copyright information about the original engine. */
	virtual const char *getOriginalCopyright() const = 0;

	/** Returns a list of games supported by this engine. */
	virtual PlainGameList getSupportedGames() const = 0;

	/** Query the engine for a PlainGameDescriptor for the specified gameid, if any. */
	virtual PlainGameDescriptor findGame(const char *gameId) const = 0;

	/**
	 * Runs the engine's game detector on the given list of files, and returns a
	 * (possibly empty) list of games supported by the engine which it was able
	 * to detect amongst the given files.
	 */
	virtual DetectedGames detectGames(const Common::FSList &fslist) const = 0;

	/**
	 * Tries to instantiate an engine instance based on the settings of
	 * the currently active ConfMan target. That is, the MetaEngine should
	 * query the ConfMan singleton for the target, gameid, path etc. data.
	 *
	 * @param syst	Pointer to the global OSystem object
	 * @param engine	Pointer to a pointer which the MetaEngine sets to
	 *					the newly create Engine, or 0 in case of an error
	 * @return		a Common::Error describing the error which occurred, or kNoError
	 */
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const = 0;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * The returned list is guaranteed to be sorted by slot numbers. That
	 * means smaller slot numbers are always stored before bigger slot numbers.
	 *
	 * The caller has to ensure that this (Meta)Engine is responsible
	 * for the specified target (by using findGame on it respectively
	 * on the associated gameid from the relevant ConfMan entry, if present).
	 *
	 * The default implementation returns an empty list.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * @param target	name of a config manager target
	 * @return			a list of save state descriptors
	 */
	virtual SaveStateList listSaves(const char *target) const;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * This is a wrapper around the basic listSaves virtual method, but which
	 * has some extra logic for autosave handling
	 *
	 * @param target	name of a config manager target
	 * @param saveMode	If true, getting the list for a save dialog
	 * @return			a list of save state descriptors
	 */
	SaveStateList listSaves(const char *target, bool saveMode) const;

	/**
	 * Returns the slot number being used for autosaves.
	 * @note	This should match the engine getAutosaveSlot() method
	 */
	virtual int getAutosaveSlot() const {
		return 0;
	}

	/**
	 * Return a list of extra GUI options for the specified target.
	 * If no target is specified, all of the available custom GUI options are
	 * Returned for the plugin (used to set default values).
	 *
	 * Currently, this only supports options with checkboxes.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target    name of a config manager target
	 * @return          a list of extra GUI options for an engine plugin and
	 *                  target
	 */
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const {
		return ExtraGuiOptions();
	}

	/**
	 * Register the default values for the settings the engine uses into the
	 * configuration manager.
	 *
	 * @param target    name of a config manager target
	 */
	virtual void registerDefaultSettings(const Common::String &target) const;

	/**
	 * Return a GUI widget container for configuring the specified target options.
	 *
	 * The returned widget is shown in the Engine tab in the edit game dialog.
	 * Engines can build custom options dialogs, but by default a simple widget
	 * allowing to configure the extra GUI options is used.
	 *
	 * Engines that don't want to have an Engine tab in the edit game dialog
	 * can return nullptr.
	 *
	 * @param boss     the widget / dialog the returned widget is a child of
	 * @param name     the name the returned widget must use
	 * @param target   name of a config manager target
	 */
	virtual GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const;

	/**
	 * Return a list of achievement descriptions for the specified target.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target    name of a config manager target
	 * @return          a list of achievement descriptions for an engine plugin
	 *                  and target
	 */
	virtual const Common::AchievementsInfo getAchievementsInfo(const Common::String &target) const {
		return Common::AchievementsInfo();
	}

	/**
	 * Return the maximum save slot that the engine supports.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * The default implementation limits the save slots to zero (0).
	 *
	 * @return			maximum save slot number supported
	 */
	virtual int getMaximumSaveSlot() const {
		return 0;
	}

	/**
	 * Remove the specified save state.
	 *
	 * For most engines this just amounts to calling _saveFileMan->removeSaveFile().
	 * Engines which keep an index file will also update it accordingly.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsDeleteSave feature flag.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state to be removed
	 */
	virtual void removeSaveState(const char *target, int slot) const;

	/**
	 * Returns meta infos from the specified save state.
	 *
	 * Depending on the MetaEngineFeatures set this can include
	 * thumbnails, save date / time, play time.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state
	 */
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	/**
	 * Returns name of the save file for given slot and optional target.
	 *
	 * @param saveGameIdx	index of the save
	 * @param target		game target. If omitted, then the engine id is used
	 */
	virtual const char *getSavegameFile(int saveGameIdx, const char *target = nullptr) const;

	/**
	 * Returns pattern for save files.
	 *
	 * @param target		game target. If omitted, then the engine id is used
	 */
	virtual const char *getSavegamePattern(const char *target = nullptr) const;

	/**
	 * Return the keymap used by the target.
	 */
	virtual Common::Array<Common::Keymap *> initKeymaps(const char *target) const;

	/** @name MetaEngineFeature flags */
	//@{

	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 */
	enum MetaEngineFeature {
		/**
		 * Listing all Save States for a given target is supported, i.e.,
		 * the listSaves() and getMaximumSaveSlot methods are implemented.
		 * Used for --list-saves support, as well as the GMM load dialog.
		 */
		kSupportsListSaves,

		/**
		 * Loading from the Launcher / command line (-x)
		 */
		kSupportsLoadingDuringStartup,

		/**
		 * Deleting Saves from the Launcher (i.e. implements the
		 * removeSaveState() method)
		 */
		kSupportsDeleteSave,

		/**
		 * Features meta infos for savestates (i.e. implements the
		 * querySaveMetaInfos method properly).
		 *
		 * Engines implementing meta infos always have to provide
		 * the following entries in the save state descriptor queried
		 * by querySaveMetaInfos:
		 * - 'is_deletable', which indicates if a given save is
		 *                   safe for deletion
		 * - 'is_write_protected', which indicates if a given save
		 *                         can be overwritten by the user.
		 *                         (note: of course you do not have to
		 *                         set this, since it defaults to 'false')
		 */
		kSavesSupportMetaInfo,

		/**
		 * Features a thumbnail in savegames (i.e. includes a thumbnail
		 * in savestates returned via querySaveMetaInfo).
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportThumbnail,

		/**
		 * Features 'save_date' and 'save_time' entries in the
		 * savestate returned by querySaveMetaInfo. Those values
		 * indicate the date/time the savegame was created.
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportCreationDate,

		/**
		 * Features 'play_time' entry in the savestate returned by
		 * querySaveMetaInfo. It indicates how long the user played
		 * the game till the save.
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportPlayTime,

		/**
		* Feature is available if engine's saves could be detected
		* with "<target>.###" pattern and "###" corresponds to slot
		* number.
		*
		* If that's not true or engine is using some unusual way
		* of detecting saves and slot numbers, this should be
		* unavailable. In that case Save/Load dialog for engine's
		* games is locked during cloud saves sync.
		*/
		kSimpleSavesNames,

		/**
		 * Uses default implementation of save header and thumbnail
		 * appended to the save.
		 * This flag requires the following flags to be set:
		 *   kSavesSupportMetaInfo, kSavesSupportThumbnail, kSavesSupportCreationDate,
		 *   kSavesSupportPlayTime
		 */
		kSavesUseExtendedFormat
	};

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 * Used by e.g. the launcher to determine whether to enable the "Load" button.
	 */
	virtual bool hasFeature(MetaEngineFeature f) const;

	static void appendExtendedSave(Common::OutSaveFile *saveFile, uint32 playtime,
		Common::String desc, bool isAutosave);
	static void parseSavegameHeader(ExtendedSavegameHeader *header, SaveStateDescriptor *desc);
	static void fillDummyHeader(ExtendedSavegameHeader *header);
	static WARN_UNUSED_RESULT bool readSavegameHeader(Common::InSaveFile *in, ExtendedSavegameHeader *header, bool skipThumbnail = true);

	//@}
};

/**
 * Singleton class which manages all Engine plugins.
 */
class EngineManager : public Common::Singleton<EngineManager> {
public:
	/**
	 * Given a list of FSNodes in a given directory, detect a set of games contained within
	 *
	 * Returns an empty list if none are found.
	 */
	DetectionResults detectGames(const Common::FSList &fslist) const;

	/** Find a plugin by its engine ID */
	const Plugin *findPlugin(const Common::String &engineId) const;

	/** Get the list of all engine plugins */
	const PluginList &getPlugins() const;

	/** Find a target */
	QualifiedGameDescriptor findTarget(const Common::String &target, const Plugin **plugin = NULL) const;

	/**
	 * List games matching the specified criteria
	 *
	 * If the engine id is not specified, this scans all the plugins,
	 * loading them from disk if necessary. This is a slow operation on
	 * some platforms and should not be used for the happy path.
	 */
	QualifiedGameList findGamesMatching(const Common::String &engineId, const Common::String &gameId) const;

	/**
	 * Create a target from the supplied game descriptor
	 *
	 * Returns the created target name.
	 */
	Common::String createTargetForGame(const DetectedGame &game);

	/** Upgrade a target to the current configuration format */
	void upgradeTargetIfNecessary(const Common::String &target) const;

private:
	/** Find a game across all loaded plugins */
	QualifiedGameList findGameInLoadedPlugins(const Common::String &gameId) const;

	/** Find a loaded plugin with the given engine ID */
	const Plugin *findLoadedPlugin(const Common::String &engineId) const;

	/** Use heuristics to complete a target lacking an engine ID */
	void upgradeTargetForEngineId(const Common::String &target) const;
};

/** Convenience shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()

#endif
