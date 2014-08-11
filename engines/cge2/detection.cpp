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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/detection.h"

namespace CGE2 {

bool CGE2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new CGE2::CGE2Engine(syst, desc);

	return desc != 0;
}

bool CGE2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
}

const ADGameDescription *CGE2MetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	static ADGameDescription desc;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		if (file->getName().equalsIgnoreCase("lang.eng")) {
			Common::File dataFile;
			if (!dataFile.open(*file))
				continue;
			
			desc.gameid = "sfinx";
			desc.extra = "Sfinx English Alpha v0.1";
			desc.language = Common::EN_ANY;
			desc.platform = Common::kPlatformDOS;
			desc.flags = ADGF_NO_FLAGS;
			desc.guioptions = GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF);
			
			return (const ADGameDescription *)&desc;
		}
	}
	return 0;
}

} // End of namespace CGE2

#if PLUGIN_ENABLED_DYNAMIC(CGE2)
REGISTER_PLUGIN_DYNAMIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#endif
