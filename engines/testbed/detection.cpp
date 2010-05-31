#include "testbed/testbed.h"
 
#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
 
#include "engines/metaengine.h"
 
static const PlainGameDescriptor testbed_setting[] = {
	{ "testbed", "TestBed: The backend testing framework" },
	{ 0, 0 }
};
 
class TestbedMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const {
		return "TestBed: The backend testing framework";
	}
 
	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) ScummVM";
	}
 
	virtual GameList getSupportedGames() const {
		GameList games;
		const PlainGameDescriptor *g = testbed_setting;
		while (g->gameid) {
			games.push_back(*g);
			g++;
		}
 
		return games;
	}
 
	virtual GameDescriptor findGame(const char *gameid) const {
		const PlainGameDescriptor *g = testbed_setting;
		while (g->gameid) {
			if (0 == scumm_stricmp(gameid, g->gameid))
				break;
			g++;
		}
		return GameDescriptor(g->gameid, g->description);
	}
 
	virtual GameList detectGames(const Common::FSList &fslist) const {
		GameList detectedGames;
 
		// Iterate over all files in the given directory
		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *gameName = file->getName().c_str();
 
				if (0 == scumm_stricmp("README", gameName)) {
					// You could check the contents of the file now if you need to.
					detectedGames.push_back(testbed_setting[0]);
					break;
				}
			}
		}
		return detectedGames;
	}
 
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const {
		assert(syst);
		assert(engine);
 
		// Scan the target directory for files (error out if it does not exist)
		Common::FSList fslist;
		Common::FSNode dir(ConfMan.get("path"));
		if (!dir.getChildren(fslist, Common::FSNode::kListAll)) {
			return Common::kInvalidPathError;
		}
 
		// Invoke the detector
		Common::String gameid = ConfMan.get("gameid");
		GameList detectedGames = detectGames(fslist);
 
		for (uint i = 0; i < detectedGames.size(); i++) {
			if (detectedGames[i].gameid() == gameid) {
				// At this point you may want to perform additional sanity checks.
				*engine = new Testbed::TestbedEngine(syst);
				return Common::kNoError;
			}
		}
 
		// Failed to find any game data
		return Common::kNoGameDataFoundError;
	}
};
 
#if PLUGIN_ENABLED_DYNAMIC(TESTBED)
	REGISTER_PLUGIN_DYNAMIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#endif
