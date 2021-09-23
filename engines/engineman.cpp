#include "engines/engineman.h"

#include "base/plugins.h"

#include "common/func.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"

#include "base/detection/detection.h"

#include "engines/advancedDetector.h"

namespace Common {
DECLARE_SINGLETON(EngineManager);
}

/**
 * This function works for both cached and uncached PluginManagers.
 * For the cached version, most of the logic here will short circuit.
 *
 * For the uncached version, we first try to find the plugin using the engineId
 * and only if we can't find it there, we loop through the plugins.
 **/
QualifiedGameList EngineManager::findGamesMatching(const Common::String &engineId, const Common::String &gameId) const {
	QualifiedGameList results;

	if (!engineId.empty()) {
		// If we got an engine name, look for THE game only in that engine
		const Plugin *p = EngineMan.findPlugin(engineId);
		if (p) {
			const MetaEngineDetection &engine = p->get<MetaEngineDetection>();
			DebugMan.addAllDebugChannels(engine.getDebugChannels());

			PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());
			if (pluginResult.gameId) {
				results.push_back(QualifiedGameDescriptor(engine.getEngineId(), pluginResult));
			}
		}
	} else {
		// This is a slow path, we have to scan the list of plugins
		PluginMan.loadFirstPlugin();
		do {
			results.push_back(findGameInLoadedPlugins(gameId));
		} while (PluginMan.loadNextPlugin());
	}

	return results;
}

/**
 * Find the game within the plugins loaded in memory
 **/
QualifiedGameList EngineManager::findGameInLoadedPlugins(const Common::String &gameId) const {
	// Find the GameDescriptor for this target
	const PluginList &plugins = getPlugins();

	QualifiedGameList results;
	PluginList::const_iterator iter;

	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &engine = (*iter)->get<MetaEngineDetection>();
		DebugMan.addAllDebugChannels(engine.getDebugChannels());
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

	// MetaEngines are always loaded into memory, so, get them and
	// run detection for all of them.
	plugins = getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	// Clear md5 cache before each detection starts, just in case.
	MD5Man.clear();

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &metaEngine = (*iter)->get<MetaEngineDetection>();
		// set the debug flags
		DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
		DetectedGames engineCandidates = metaEngine.detectGames(fslist);

		for (uint i = 0; i < engineCandidates.size(); i++) {
			engineCandidates[i].path = fslist.begin()->getParent().getPath();
			engineCandidates[i].shortPath = fslist.begin()->getParent().getDisplayName();
			candidates.push_back(engineCandidates[i]);
		}
	}

	return DetectionResults(candidates);
}


const PluginList &EngineManager::getPlugins(const PluginType fetchPluginType) const {
	return PluginManager::instance().getPlugins(fetchPluginType);
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

const Plugin *EngineManager::findPlugin(const Common::String &engineId) const {
	const PluginList &plugins = getPlugins();

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		if (engineId == (*iter)->get<MetaEngineDetection>().getEngineId())
			return *iter;

	return 0;
}

QualifiedGameDescriptor EngineManager::findTarget(const Common::String &target, const Plugin **plugin) const {
	// Ignore empty targets
	if (target.empty())
		return QualifiedGameDescriptor();

	// Lookup the domain. If we have no domain, fallback on the old function [ultra-deprecated].
	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	if (!domain || !domain->contains("gameid") || !domain->contains("engineid"))
		return QualifiedGameDescriptor();

	// Look for the engine ID
	const Plugin *foundPlugin = findPlugin(domain->getVal("engineid"));
	if (!foundPlugin) {
		return QualifiedGameDescriptor();
	}

	// Make sure it does support the game ID
	const MetaEngineDetection &engine = foundPlugin->get<MetaEngineDetection>();
	DebugMan.addAllDebugChannels(engine.getDebugChannels());
	PlainGameDescriptor desc = engine.findGame(domain->getVal("gameid").c_str());
	if (!desc.gameId) {
		return QualifiedGameDescriptor();
	}

	if (plugin)
		*plugin = foundPlugin;

	return QualifiedGameDescriptor(engine.getEngineId(), desc);
}

void EngineManager::upgradeTargetIfNecessary(const Common::String &target) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	if (!domain->contains("engineid")) {
		upgradeTargetForEngineId(target);
	} else {
		if (domain->getVal("engineid").equals("fullpipe")) {
			domain->setVal("engineid", "ngi");

			debug("Upgrading engineid from 'fullpipe' to 'ngi'");

			ConfMan.flushToDisk();
		}
	}
}

void EngineManager::upgradeTargetForEngineId(const Common::String &target) const {
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
	const Plugin *plugin = findPlugin(oldGameId);
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
		// set debug flags before call detectGames
		DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
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
