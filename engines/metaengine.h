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
 * @defgroup engines_metaengine Meta engine
 * @ingroup engines
 *
 * @brief API for managing various functions of the meta engine.
 *
 * @{
 */

/**
 * Per-game extra GUI options structure.
 * Currently, this can only be used for options with checkboxes.
 */
struct ExtraGuiOption {
	const char *label;         /*!< Option label, e.g. "Fullscreen mode". */
	const char *tooltip;       /*!< Option tooltip shown when the mouse cursor hovers over it. */
	const char *configOption;  /*!< confMan key, e.g. "fullscreen". */
	bool defaultState;         /*!< Default state of the checkbox (checked or not). */
};

/**
 * Array of ExtraGuiOption structures.
 */
typedef Common::Array<ExtraGuiOption> ExtraGuiOptions;

/**
 * @todo Doc required
 */
enum { kSavegameFilePattern = -99 };

#define EXTENDED_SAVE_VERSION 4

/**
 * Structure describing a savegame file.
 */
struct ExtendedSavegameHeader {
	char id[6];                   /*!< ID of the savegame file. */
	uint8 version;                /*!< Version of the savegame header. */
	Common::String saveName;      /*!< Name of the savegame. */
	Common::String description;   /*!< Description of the savegame, as entered by the user. */
	uint32 date;                  /*!< Date of the savegame. */
	uint16 time;                  /*!< Time of the savegame. */
	uint32 playtime;              /*!< Total play time until this savegame. */
	Graphics::Surface *thumbnail; /*!< Screen content shown as a thumbnail for this savegame. */
	bool isAutosave;              /*!< Whether this savegame is an autosave. */

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
 * A meta engine factory for Engine instances with the
 * added ability of listing and detecting supported games.
 *
 * Every engine "plugin" provides a hook to get an instance of a MetaEngineDetection
 * subclass for that "engine plugin". For example, SCUMM provides ScummMetaEngineDetection.
 * This is then in turn used by the frontend code to detect games,
 * and other useful functionality. 
 *
 * To instantiate actual Engine objects, see the class @ref MetaEngine.
 */
class MetaEngineDetection : public PluginObject {
public:
	virtual ~MetaEngineDetection() {}

	/** Get the engine ID. */
	virtual const char *getEngineId() const = 0;

	/** Return some copyright information about the original engine. */
	virtual const char *getOriginalCopyright() const = 0;

	/** Return a list of games supported by this engine. */
	virtual PlainGameList getSupportedGames() const = 0;

	/** Query the engine for a PlainGameDescriptor for the specified gameid, if any. */
	virtual PlainGameDescriptor findGame(const char *gameId) const = 0;

	/**
	 * Run the engine's game detector on the given list of files, and return a
	 * (possibly empty) list of games supported by the engine that were
	 * found among the given files.
	 */
	virtual DetectedGames detectGames(const Common::FSList &fslist) const = 0;

	/**
	 * Return a list of extra GUI options for the specified target.
	 *
	 * If no target is specified, all of the available custom GUI options are
	 * returned for the plugin (used to set default values).
	 *
	 * Currently, this only supports options with checkboxes.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target  Name of a config manager target.
	 *
	 * @return A list of extra GUI options for an engine plugin and target.          
	 */
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const {
		return ExtraGuiOptions();
	}

	/**
	 * Register the default values for the settings that the engine uses into the
	 * configuration manager.
	 *
	 * @param target  Name of a config manager target.
	 */
	virtual void registerDefaultSettings(const Common::String &target) const;

	/**
	 * Return a GUI widget container for configuring the specified target options.
	 *
	 * The returned widget is shown in the Engine tab in the Edit Game dialog.
	 * Engines can build custom option dialogs, but by default a simple widget
	 * allowing to configure the extra GUI options is used.
	 *
	 * Engines that are not supposed to have an Engine tab in the Edit Game dialog
	 * can return nullptr.
	 *
	 * @param boss     The widget or dialog that the returned widget is a child of.
	 * @param name     The name that the returned widget must use.
	 * @param target   Name of a config manager target.
	 */
	virtual GUI::OptionsContainerWidget *buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const;
};

/**
 * A MetaEngine is another factory for Engine instances, and is very
 * similar to meta engines.
 *
 * This class, however, is made of of bridged functionalities that can be used to connect
 * an actual Engine with a MetaEngine. Every engine "plugin" provides a hook to get an instance
 * of the MetaEngine subclass for that "engine plugin.". For example, SCUMM provides a ScummMetaEngine.
 * This is then in turn used for things like instantiating engine objects, listing savefiles,
 * querying save metadata, etc.
 *
 * Since engine plugins can use external runtime libraries, these can live and build inside
 * the engine, while a MetaEngine will always build into the executable to be able to detect code.
 */
class MetaEngine : public PluginObject {
private:
	/**
	 * Convert the current screen contents to a thumbnail and save it.
	 */
	static void saveScreenThumbnail(Common::OutSaveFile *saveFile);
public:
	virtual ~MetaEngine() {}

	/**
	 * Name of the engine plugin.
	 *
	 * Classes inheriting a MetaEngineConnect must provide an engineID here,
	 * which can then be used to match an Engine with MetaEngine.
	 *
	 * For example, ScummMetaEngine inherits MetaEngine and provides a engineID of "Scumm".
	 * ScummMetaEngineConnect inherits MetaEngineConnect and provides the name "Scumm".
	 * This way, an Engine can be easily matched with a MetaEngine.
	 */
	virtual const char *getName() const = 0;

	/**
	 * Instantiate an engine instance based on the settings of
	 * the currently active ConfMan target.
	 *
	 * The MetaEngine queries the ConfMan singleton for data like the target,
	 * gameid, path etc.
	 *
	 * @param syst    Pointer to the global OSystem object.
	 * @param engine  Pointer to a pointer that the MetaEngine sets to
	 *                the newly created Engine, or 0 in case of an error.
	 *
	 * @return A Common::Error describing the error that occurred, or kNoError.
	 */
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const = 0;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * The returned list is guaranteed to be sorted by slot numbers. That
	 * means smaller slot numbers are always stored before bigger slot numbers.
	 *
	 * The caller must ensure that this (Meta)Engine is responsible
	 * for the specified target. This is done by using findGame on it respectively
	 * on the associated gameid from the relevant ConfMan entry, if present.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * @param target  Name of a config manager target.
	 *
	 * @return A list of save state descriptors.
	 */
	virtual SaveStateList listSaves(const char *target) const;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * This is a wrapper around the basic listSaves virtual method, but it has
	 * some extra logic for autosave handling.
	 *
	 * @param target    Name of a config manager target.
	 * @param saveMode  If true, get the list for a save dialog.
	 * @return A list of save state descriptors.
	 */
	SaveStateList listSaves(const char *target, bool saveMode) const;

	/**
	 * Return the slot number that is used for autosaves.
	 *
	 * @note This should match the engine getAutosaveSlot() method.
	 */
	virtual int getAutosaveSlot() const {
		return 0;
	}

	/**
	 * Return the maximum number of save slots that the engine supports.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * The default implementation limits the save slots to zero (0).
	 *
	 * @return Maximum save slot number supported.
	 */
	virtual int getMaximumSaveSlot() const {
		// For games using the new save format, assume 99 slots by default
		return hasFeature(kSavesUseExtendedFormat) ? 99 : 0;
	}

	/**
	 * Remove the specified save state.
	 *
	 * For most engines, this just means calling _saveFileMan->removeSaveFile().
	 * Engines that keep an index file will also update it accordingly.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsDeleteSave feature flag.
	 *
	 * @param target  Name of a config manager target.
	 * @param slot    Slot number of the save state to be removed.
	 */
	virtual void removeSaveState(const char *target, int slot) const;

	/**
	 * Return meta information from the specified save state.
	 *
	 * Depending on the MetaEngineFeatures set, this can include
	 * thumbnails, save date and time, play time.
	 *
	 * @param target  Name of a config manager target.
	 * @param slot    Slot number of the save state.
	 */
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	/**
	 * Return the name of the save file for the given slot and optional target,
	 * or a pattern for matching filenames against.
	 *
	 * @param saveGameIdx  Index of the save, or kSavegameFilePattern
	 *                     for returning a filename pattern.
	 * @param target       Game target. If omitted, then the engine ID is used.
	 */
	virtual Common::String getSavegameFile(int saveGameIdx, const char *target = nullptr) const;

	/**
	 * Return the pattern for save files.
	 *
	 * @param target  Game target. If omitted, then the engine ID is used.
	 */
	Common::String getSavegameFilePattern(const char *target = nullptr) const {
		return getSavegameFile(kSavegameFilePattern, target);
	}

	/**
	 * Return the keymap used by the target.
	 */
	virtual Common::Array<Common::Keymap *> initKeymaps(const char *target) const;

	/**
	 * Return the extra GUI options used by the target.
	 */
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const {
		return ExtraGuiOptions();
	}

	/**
	 * Return a GUI widget container for configuring the specified target options.
	 *
	 * Engines can build custom option dialogs from here, but by default a simple widget
	 * allowing to configure the extra GUI options is used.
	 *
	 * The engine that builds the Engines tab in the Edit Game dialog uses a MetaEngine.
	 * The engine that specifies a custom dialog when a game is running uses a MetaEngineConnect.
	 *
	 * Engines are not supposed to have an Engine tab in the Edit Game dialog
	 * can return nullptr.
	 *
	 * @param boss    The widget or dialog that the returned widget is a child of.
	 * @param name    The name that the returned widget must use.
	 * @param target  Name of a config manager target.
	 */
	virtual GUI::OptionsContainerWidget *buildEngineOptionsWidgetDynamic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const;

	/**
	 * MetaEngine feature flags.
	 *
	 * A feature in this context means an ability of the engine that can be
	 * either available or not.
	 */
	enum MetaEngineFeature {
		/**
		 * List all save states for a given target that is supported.
		 *
		 * For the target to be supported, the listSaves() and
		 * getMaximumSaveSlot methods must be implemented.
		 * Used for --list-saves support, as well as the GMM load dialog.
		 */
		kSupportsListSaves,

		/**
		 * Load from the launcher or command line (-x).
		 */
		kSupportsLoadingDuringStartup,

		/**
		 * Delete saves from the launcher.
		 *
		 * This means that the engine implements the removeSaveState() method.
		 */
		kSupportsDeleteSave,

		/**
		 * Feature meta information for save states.
		 *
		 * This means that the engine implements the querySaveMetaInfos method properly.
		 *
		 * Engines implementing meta information always must provide
		 * the following entries in the save state descriptor queried
		 * by querySaveMetaInfos:
		 * - is_deletable - indicates whether a given save is
		 *                  safe for deletion
		 * - is_write_protected - indicates whether a given save
		 *                        can be overwritten by the user.
		 *                        You do not need to set this, default value is 'false'.
		 */
		kSavesSupportMetaInfo,

		/**
		 * Feature a thumbnail in savegames.
		 *
		 * This means that the engine includes a thumbnail in save states 
		 * returned via querySaveMetaInfo.
		 * This flag can only be set when kSavesSupportMetaInfo is set.
		 */
		kSavesSupportThumbnail,

		/**
		 * Feature @c save_date and @c save_time entries in the
		 * save state returned by querySaveMetaInfo.
		 *
		 * These values indicate the date and time when the savegame was created.
		 * This flag can only be set when kSavesSupportMetaInfo is set.
		 */
		kSavesSupportCreationDate,

		/**
		 * Feature @c play_time entry in the save state returned by
		 * querySaveMetaInfo.
		 *
		 * This indicates how long the user played the game until the save.
		 * This flag can only be set when kSavesSupportMetaInfo is set.
		 */
		kSavesSupportPlayTime,

		/**
		* This feature is available if the engine's saves can be detected
		* with:
		* @code
		* <target>.###
		* @endcode
		* where ### corresponds to slot number.
		*
		* If that is not true, or if the engine uses some unusual way
		* of detecting saves and slot numbers, this should be
		* unavailable. In that case Save/Load dialog for the engine's
		* games is locked during cloud saves sync.
		*/
		kSimpleSavesNames,

		/**
		 * Use the default implementation of save header and thumbnail
		 * appended to the save.
		 *
		 * This flag requires the following flags to be set:
		 * - kSavesSupportMetaInfo
		 * - kSavesSupportThumbnail
		 * - kSavesSupportCreationDate
		 * - kSavesSupportPlayTime
		 */
		kSavesUseExtendedFormat
	};

	/**
	 * Return a list of achievement descriptions for the specified target.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target  Name of a config manager target.
	 *
	 * @return A list of achievement descriptions for an engine plugin and target.                
	 */
	virtual const Common::AchievementsInfo getAchievementsInfo(const Common::String &target) const {
		return Common::AchievementsInfo();
	}

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	virtual bool hasFeature(MetaEngineFeature f) const;

	/**
	 * Write the extended savegame header to the given savegame file.
	 */
	static void appendExtendedSave(Common::OutSaveFile *saveFile, uint32 playtime, Common::String desc, bool isAutosave);
	/**
	 * Parse the extended savegame header to retrieve the SaveStateDescriptor information.
	 */
	static void parseSavegameHeader(ExtendedSavegameHeader *header, SaveStateDescriptor *desc);
	/**
	 * Populate the given extended savegame header with dummy values.
	 *
	 * This is used when failing to read the header from a savegame file.
	 */
	static void fillDummyHeader(ExtendedSavegameHeader *header);
	/**
	 * Read the extended savegame header from the given savegame file.
	 */
	static WARN_UNUSED_RESULT bool readSavegameHeader(Common::InSaveFile *in, ExtendedSavegameHeader *header, bool skipThumbnail = true);
};

/**
 * Singleton class that manages all engine plugins.
 */
class EngineManager : public Common::Singleton<EngineManager> {
public:
	/**
	 * Given a list of FSNodes in a given directory, detect a set of games contained within.
	 *
	 * Returns an empty list if none are found.
	 */
	DetectionResults detectGames(const Common::FSList &fslist) const;

	/** Find a plugin by its engine ID. */
	const Plugin *findPlugin(const Common::String &engineId) const;

	/**
	 * Get the list of all plugins for the type specified.
	 *
	 * By default, it will get METAENGINES, for now.
	 * If usage of actual engines never occurs, the default arguments can be skipped,
	 * and always have it return PLUGIN_TYPE_ENGINE_DETECTION.
	 */
	const PluginList &getPlugins(const PluginType fetchPluginType = PLUGIN_TYPE_ENGINE_DETECTION) const;

	/** Find a target. */
	QualifiedGameDescriptor findTarget(const Common::String &target, const Plugin **plugin = NULL) const;

	/**
	 * List games matching the specified criteria.
	 *
	 * If the engine ID is not specified, this scans all the plugins,
	 * loading them from the disk if necessary. This is a slow operation on
	 * some platforms and should not be used for the happy path.
	 */
	QualifiedGameList findGamesMatching(const Common::String &engineId, const Common::String &gameId) const;

	/**
	 * Create a target from the supplied game descriptor.
	 *
	 * @return The created target name.
	 */
	Common::String createTargetForGame(const DetectedGame &game);

	/** Upgrade a target to the current configuration format. */
	void upgradeTargetIfNecessary(const Common::String &target) const;

private:
	/** Find a game across all loaded plugins. */
	QualifiedGameList findGameInLoadedPlugins(const Common::String &gameId) const;

	/** Find a loaded plugin with the given engine ID. */
	const Plugin *findLoadedPlugin(const Common::String &engineId) const;

	/** Use heuristics to complete a target lacking an engine ID. */
	void upgradeTargetForEngineId(const Common::String &target) const;
};

/** Convenience shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()
/** @} */
#endif
