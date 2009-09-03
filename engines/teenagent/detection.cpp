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


#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "teenagent/teenagent.h"
#include "common/system.h"
#include "common/savefile.h"
#include "teenagent.h"

static const PlainGameDescriptor teenAgentGames[] = {
	{ "teenagent", "Teen agent" },
	{ 0, 0 }
};


static const TeenAgent::GameDescription teenAgentGameDescriptions[] = {
	{
		{
			"teenagent",
			"",
			AD_ENTRY1s("teenagnt.exe", "b886cae8f875ea5eaefed04a8cc3c8a1", 152690),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		}, 
		{0x0200, 0xb5b0, 0x1c890}
	},
/*	
	{
		{
			"teenagent",
			"",
			AD_ENTRY1s("teenagnt.exe", "7172e0c46cd11e4072ba486e3d220210", 152626),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		{0x00c0, 0xB5E0, 0x1c850}
	}, 
*/	
	{
		AD_TABLE_END_MARKER, 
		{0, 0, 0}
	}
};

static const ADParams detectionParams = {
	(const byte *)teenAgentGameDescriptions,
	sizeof(TeenAgent::GameDescription),
	5000,
	teenAgentGames,
	0,
	"teenagent",
	0,
	0,
	Common::GUIO_NONE
};

#define MAX_SAVES 20

class TeenAgentMetaEngine : public AdvancedMetaEngine {
public:
	TeenAgentMetaEngine() : AdvancedMetaEngine(detectionParams) {
	}

	virtual const char *getName() const {
		return "Teen Agent Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "TEENAGENT demo and TEENAGENT name copyright (c) Metropolis 1994.";
	}

	virtual bool hasFeature(MetaEngineFeature f) const {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsDeleteSave:
		case kSupportsLoadingDuringStartup:
		//case kSavesSupportThumbnail:
			return true;
		default:
			return false;
		}
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if (desc) {
			*engine = new TeenAgent::TeenAgentEngine(syst, (TeenAgent::GameDescription*)desc);
		}
		return desc != 0;
	}

//	virtual const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const {
//		return 0;
//	}

	static Common::String generateGameStateFileName(const char *target, int slot) {
		char slotStr[16];
		snprintf(slotStr, sizeof(slotStr), ".%d", slot);
		return slotStr;
	}

	virtual SaveStateList listSaves(const char *target) const {
		Common::String pattern = target;
		pattern += ".*";
		
		Common::StringList filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		bool slotsTable[MAX_SAVES];
		memset(slotsTable, 0, sizeof(slotsTable));
		SaveStateList saveList;
		for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot;
			const char *ext = strrchr(file->c_str(), '.');
			if (ext && (slot = atoi(ext + 1)) >= 0 && slot < MAX_SAVES) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);
				if (in) {
					slotsTable[slot] = true;
					delete in;
				}
			}
		}
		for (int slot = 0; slot < MAX_SAVES; ++slot) {
			if (slotsTable[slot]) {
				char description[64];
				snprintf(description, sizeof(description), "teenagent.%02d", slot);
				saveList.push_back(SaveStateDescriptor(slot, description));
			}
		}
		return saveList;
	}

	virtual int getMaximumSaveSlot() const {
		return MAX_SAVES - 1;
	}

	virtual void removeSaveState(const char *target, int slot) const {
		Common::String filename = generateGameStateFileName(target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}
};

#if PLUGIN_ENABLED_DYNAMIC(TEENAGENT)
	REGISTER_PLUGIN_DYNAMIC(TEENAGENT, PLUGIN_TYPE_ENGINE, TeenAgentMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TEENAGENT, PLUGIN_TYPE_ENGINE, TeenAgentMetaEngine);
#endif
