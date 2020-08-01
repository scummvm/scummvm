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

#include "common/config-manager.h"
#include "common/debug.h"

#include "engines/metaengine.h"
#include "engines/engineman.h"

namespace Common {
DECLARE_SINGLETON(EngineManager);
}

QualifiedGameList EngineManager::findGamesMatching(const Common::String &engineId, const Common::String &gameId) const {
	// Find the GameDescriptor for this target
	const PluginList &plugins = getEngineDetectionPlugins();

	QualifiedGameList results;
	PluginList::const_iterator iter;

	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &engine = (*iter)->get<MetaEngineDetection>();
		PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());

		if (pluginResult.gameId) {
			results.push_back(QualifiedGameDescriptor(engine.getEngineId(), pluginResult));
		}
	}

	return results;
}

DetectionResults EngineManager::detectGames(const Common::FSList &fslist) const {
	DetectedGames candidates;
	PluginList plugins;
	PluginList::const_iterator iter;

	plugins = getEngineDetectionPlugins();
	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &metaEngine = (*iter)->get<MetaEngineDetection>();
		DetectedGames engineCandidates = metaEngine.detectGames(fslist);

		for (uint i = 0; i < engineCandidates.size(); i++) {
			engineCandidates[i].path = fslist.begin()->getParent().getPath();
			engineCandidates[i].shortPath = fslist.begin()->getParent().getDisplayName();
			candidates.push_back(engineCandidates[i]);
		}
	}

	return DetectionResults(candidates);
}

const PluginList EngineManager::getEnginePlugins() const {
	return PluginMan.getAllPluginsOfType(PLUGIN_TYPE_ENGINE);
}

const PluginList EngineManager::getEngineDetectionPlugins() const {
	return PluginMan.getAllPluginsOfType(PLUGIN_TYPE_ENGINE_DETECTION);
}

namespace {

void addStringToConf(const Common::String &key, const Common::String &value, const Common::String &domain) {
	if (!value.empty())
		ConfMan.set(key, value, domain);
}

} // End of anonymous namespace

Common::String EngineManager::createTargetForGame(const DetectedGame &game) {
	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	Common::String domain = game.preferredTarget;

	assert(!domain.empty());
	if (ConfMan.hasGameDomain(domain)) {
		int suffixN = 1;
		Common::String gameid(domain);

		while (ConfMan.hasGameDomain(domain)) {
			domain = gameid + Common::String::format("-%d", suffixN);
			suffixN++;
		}
	}

	// Add the name domain
	ConfMan.addGameDomain(domain);

	// Copy all non-empty relevant values into the new domain
	addStringToConf("engineid", game.engineId, domain);
	addStringToConf("gameid", game.gameId, domain);
	addStringToConf("description", game.description, domain);
	addStringToConf("language", Common::getLanguageCode(game.language), domain);
	addStringToConf("platform", Common::getPlatformCode(game.platform), domain);
	addStringToConf("path", game.path, domain);
	addStringToConf("extra", game.extra, domain);
	addStringToConf("guioptions", game.getGUIOptions(), domain);

	// Add any extra configuration keys
	for (Common::StringMap::iterator i = game._extraConfigEntries.begin();
	     i != game._extraConfigEntries.end(); ++i)
		addStringToConf((*i)._key, (*i)._value, domain);

	// TODO: Setting the description field here has the drawback
	// that the user does never notice when we upgrade our descriptions.
	// It might be nice to leave this field empty, and only set it to
	// a value when the user edits the description string.
	// However, at this point, that's impractical. Once we have a method
	// to query all backends for the proper & full description of a given
	// game target, we can change this (currently, you can only query
	// for the generic gameid description; it's not possible to obtain
	// a description which contains extended information like language, etc.).

	return domain;
}

const Plugin *EngineManager::findLoadedPluginByEngine(const Common::String &engineId) const {
	const PluginList &plugins = PluginMan.getLoadedPluginsOfType(PLUGIN_TYPE_ENGINE);

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		if (engineId == (*iter)->get<MetaEngine>().getEngineId())
			return *iter;

	return 0;
}

const Plugin *EngineManager::findPluginForEngine(const Common::String &engineId) {
	// First look for the game using the plugins in memory. This is critical
	// for calls coming from inside games
	const Plugin *plugin = findLoadedPluginByEngine(engineId);
	if (plugin)
		return plugin;

	// Now look for the plugin using the engine ID. This is much faster than scanning plugin
	// by plugin
	plugin = loadPluginFromEngineId(engineId);
	if (plugin)
		return plugin;

	return 0;
}

/**
 * Try to load the plugin by searching in the ConfigManager for a matching
 * engine ID under the domain 'engine_plugin_files'.
 **/
bool EngineManager::getFileNameFromEngineIdCached(const Common::String &engineId, Common::String &out) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain("engine_plugin_files");

	if (domain) {
		return domain->tryGetVal(engineId, out);
	}
	return false;
}

Plugin *EngineManager::loadPluginFromEngineId(const Common::String &engineId) {
	Plugin *plugin = getPluginByEngineIdCached(engineId);
	if (plugin) {
		if (PluginMan.loadPlugin(plugin)) {
			return plugin;
		} else {
			return nullptr;
		}
	}
	
	return loadPluginByEngineIdUncached(engineId);
}

Plugin *EngineManager::getPluginByEngineIdCached(const Common::String &engineId) {
	Common::String fileName;
	if (!getFileNameFromEngineIdCached(engineId, fileName))
		return nullptr;
	return PluginMan.getPluginByFileName(fileName);
}

Plugin *EngineManager::loadPluginByEngineIdUncached(const Common::String &engineId) {
	const PluginList &plugins = getEnginePlugins();

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++) {
		Plugin *p = *iter;

		bool loaded = p->isLoaded();
		if (!loaded) {
			PluginMan.loadPlugin(p);
		}

		if (engineId == p->get<MetaEngine>().getEngineId())
			return p;

		if (!loaded) {
			PluginMan.unloadPlugin(p);
		}
	}
	return nullptr;
}

QualifiedGameDescriptor EngineManager::findTarget(const Common::String &target, const Plugin **plugin) {
	// Ignore empty targets
	if (target.empty())
		return QualifiedGameDescriptor();

	// Lookup the domain. If we have no domain, fallback on the old function [ultra-deprecated].
	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	if (!domain || !domain->contains("gameid") || !domain->contains("engineid"))
		return QualifiedGameDescriptor();

	// Look for the engine ID
	const Plugin *foundPlugin = findPluginForEngine(domain->getVal("engineid"));
	if (!foundPlugin)
		return QualifiedGameDescriptor();

	// Make sure it does support the game ID
	const MetaEngineDetection &engine = foundPlugin->get<MetaEngineDetection>();
	PlainGameDescriptor desc = engine.findGame(domain->getVal("gameid").c_str());
	if (!desc.gameId) {
		return QualifiedGameDescriptor();
	}

	if (plugin)
		*plugin = foundPlugin;

	return QualifiedGameDescriptor(engine.getEngineId(), desc);
}

void EngineManager::upgradeTargetIfNecessary(const Common::String &target) {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	if (!domain->contains("engineid")) {
		upgradeTargetForEngineId(target);
	}
}

void EngineManager::upgradeTargetForEngineId(const Common::String &target) {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	debug("Target '%s' lacks an engine ID, upgrading...", target.c_str());

	Common::String oldGameId = domain->getVal("gameid");
	Common::String path = domain->getVal("path");

	// At this point the game ID and game path must be known
	if (oldGameId.empty()) {
		warning("The game ID is required to upgrade target '%s'", target.c_str());
		return;
	}
	if (path.empty()) {
		warning("The game path is required to upgrade target '%s'", target.c_str());
		return;
	}

	// Game descriptor for the upgraded target
	Common::String engineId;
	Common::String newGameId;

	// First, try to update entries for engines that previously used the "single id" system
	// Search for an engine whose ID is the game ID
	const Plugin *plugin = findPluginForEngine(oldGameId);
	if (plugin) {
		// Run detection on the game path
		Common::FSNode dir(path);
		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			warning("Failed to access path '%s' when upgrading target '%s'", path.c_str(), target.c_str());
			return;
		}

		// Take the first detection entry
		const MetaEngineDetection &metaEngine = plugin->get<MetaEngineDetection>();
		DetectedGames candidates = metaEngine.detectGames(files);
		if (candidates.empty()) {
			warning("No games supported by the engine '%s' were found in path '%s' when upgrading target '%s'",
			        metaEngine.getEngineId(), path.c_str(), target.c_str());
			return;
		}

		engineId = candidates[0].engineId;
		newGameId = candidates[0].gameId;
	}

	// Next, try to find an engine with the game ID in its supported games list
	if (engineId.empty()) {
		QualifiedGameList candidates = findGamesMatching("", oldGameId);
		if (candidates.size() > 1) {
			warning("Multiple matching engines were found when upgrading target '%s'", target.c_str());
			return;
		} else if (!candidates.empty()) {
			engineId = candidates[0].engineId;
			newGameId = candidates[0].gameId;
		}
	}

	if (engineId.empty() || newGameId.empty()) {
		warning("No matching engine was found when upgrading target '%s'", target.c_str());
		return;
	}

	domain->setVal("engineid", engineId);
	domain->setVal("gameid", newGameId);

	// Save a backup of the pre-upgrade gameId to the config file
	if (newGameId != oldGameId) {
		domain->setVal("oldgameid", oldGameId);
	}

	debug("Upgrade complete (engine ID '%s', game ID '%s')", engineId.c_str(), newGameId.c_str());

	ConfMan.flushToDisk();
}

/**
 * Update the config manager with a plugin file name that we found can handle
 * the engine.
 **/
void EngineManager::updateConfigWithFileName(const Plugin *plugin, const Common::String &engineId) {
	// Check if we have a filename for the current plugin
	if (plugin->getFileName()) {
		if (!ConfMan.hasMiscDomain("engine_plugin_files"))
			ConfMan.addMiscDomain("engine_plugin_files");

		Common::ConfigManager::Domain *domain = ConfMan.getDomain("engine_plugin_files");
		assert(domain);
		(*domain)[engineId] = plugin->getFileName();

		ConfMan.flushToDisk();
	}
}

Plugin *EngineManager::getEngineFromMetaEngine(const Plugin *plugin) {
	assert(plugin->getType() == PLUGIN_TYPE_ENGINE_DETECTION);

	Plugin *enginePlugin = nullptr;

	// Use the engineID from MetaEngine for comparasion.
	Common::String metaEnginePluginName = plugin->getEngineId();
	PluginList pl = PluginMan.getAllPluginsOfType(PLUGIN_TYPE_ENGINE);
	// Iterate over all engine plugins.
	for (PluginList::const_iterator itr = pl.begin(); itr != pl.end(); itr++) {
		// The getName() provides a name which is similiar to getEngineId.
		// Because engines are engines themselves, this function is simply named getName.
		Common::String enginePluginName((*itr)->getName());

		if (metaEnginePluginName.equalsIgnoreCase(enginePluginName)) {
			enginePlugin = (*itr);
			break;
		}
	}

	if (enginePlugin) {
		debug(9, "MetaEngine: %s \t matched to \t Engine: %s", plugin->getName(), enginePlugin->getFileName());
		return enginePlugin;
	}

	debug(9, "MetaEngine: %s couldn't find a match for an engine plugin.", plugin->getName());
	return nullptr;
}

Plugin *EngineManager::getMetaEngineFromEngine(const Plugin *plugin) {
	assert(plugin->getType() == PLUGIN_TYPE_ENGINE);

	Plugin *metaEngine = nullptr;

	PluginList pl = PluginMan.getAllPluginsOfType(PLUGIN_TYPE_ENGINE_DETECTION);

	// This will return a name of the Engine plugin, which will be identical to
	// a getEngineID from a relevant MetaEngine.
	Common::String enginePluginName(plugin->getName());

	for (PluginList::const_iterator itr = pl.begin(); itr != pl.end(); itr++) {
		Common::String metaEngineName = (*itr)->getEngineId();

		if (metaEngineName.equalsIgnoreCase(enginePluginName)) {
			metaEngine = (*itr);
			break;
		}
	}

	if (metaEngine) {
		debug(9, "Engine: %s matched to MetaEngine: %s", plugin->getFileName(), metaEngine->getName());
		return metaEngine;
	}

	debug(9, "Engine: %s couldn't find a match for an MetaEngine plugin.", plugin->getFileName());
	return nullptr;
}
