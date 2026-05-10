/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIASTATION_PROFILE_H
#define MEDIASTATION_PROFILE_H

#include "common/str.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/file.h"

namespace MediaStation {

class Actor;

struct ProfileContextInfo {
	// This is usually "Document", "Context", or "Screen".
	Common::String type;
	Common::String name;
	uint unk1 = 0;
};

// These can be actors or functions, hence using the generic term "asset" here.
struct ProfileAssetInfo {
	Common::String name;
	uint id = 0;
	Common::Array<uint16> channelIdents;
};

struct ProfileFileInfo {
	Common::String name;
	uint id = 0;
};

struct ProfileVariableInfo {
	Common::String name;
	uint id = 0;
};

struct ProfileParamTokenInfo {
	Common::String name;
	uint id = 0;
};

struct ProfileScriptConstantInfo {
	Common::String name;
	Common::String value;
};

// Profiles (PROFILE._ST) contain mappings between names and IDs for assets,
// functions, variables, and other entities. Some titles do not have this mapping,
// but when it exists it is very helpful for debugging. It is not required for actually
// running any games.
class Profile {
public:
	void load();

	Common::String formatActorName(uint actorId, bool attemptToGetType = false);
	Common::String formatActorName(const Actor *actor);

	Common::String formatFunctionName(uint assetId);
	Common::String formatFileName(uint fileId);
	Common::String formatVariableName(uint variableId);
	Common::String formatParamTokenName(uint paramToken);
	Common::String formatAssetNameForChannelIdent(uint channelIdent);

	const Common::String &getFileName(uint16 fileId) const { return _files.getValOrDefault(fileId).name; }
	const Common::String &getResourceName(uint16 resourceId) const { return _paramTokens.getValOrDefault(resourceId).name; }

private:
	Common::String _versionNumber;
	Common::String _platform;
	Common::Array<ProfileContextInfo> _contexts; // It isn't clear what the key would be.
	Common::HashMap<uint, ProfileAssetInfo> _assets;
	Common::HashMap<uint, uint> _channelIdentsAsIntToAssetId;
	Common::HashMap<uint, ProfileFileInfo> _files;
	Common::HashMap<uint, ProfileVariableInfo> _variables;
	Common::HashMap<uint, ProfileParamTokenInfo> _paramTokens;
	Common::Array<ProfileScriptConstantInfo> _constants;

	void parseVersionInfo(const Common::String &line);
	void parseContextInfo(const Common::String &line);
	void parseAssetInfo(const Common::String &line);
	void parseFileInfo(const Common::String &line);
	void parseVariableInfo(const Common::String &line);
	void parseParamTokenInfo(const Common::String &line);
	void parseScriptConstantInfo(const Common::String &line);

	void readSection(Common::File &file, void (Profile::*parser)(const Common::String &));
};

} // End of namespace MediaStation

#endif
