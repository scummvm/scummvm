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

#include "common/tokenizer.h"

#include "mediastation/actor.h"
#include "mediastation/debugchannels.h"
#include "mediastation/profile.h"
#include "mediastation/mediastation.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

void Profile::readSection(Common::File &file, void (Profile::*sectionParserMethod)(const Common::String &)) {
	const Common::String IMAGE_SET_LINE_DELIMITER = Common::String('#');
	const Common::String SUMMARY_LINE_DELIMITER = Common::String('*');
	const Common::String SECTION_DELIMITER = Common::String('!');

	Common::String line;
	bool shouldKeepReading = !file.err() && !file.eos();
	while (shouldKeepReading) {
		line = file.readLine();
		if (file.err() || file.eos()) {
			break;
		}

		line.trim();
		if (line.empty()) {
			// There should only be a single empty line at the end of the file.
			if (!file.eos()) {
				warning("%s: Encountered unexpected empty line", __func__);
			}

		} else if (line == SECTION_DELIMITER) {
			debugC(7, kDebugLoading, "%s: End of section", __func__);
			break;

		} else if (line.hasPrefix(SUMMARY_LINE_DELIMITER)) {
			// Example: "* 3987 20432 3881 15000"
			// Don't bother parsing the summary line, because it doesn't contain information
			// that uniquely identifies entities for pretty printing while debugging.
			debugC(7, kDebugLoading, "%s: summary: %s", __func__, line.c_str());

		} else if (line.hasPrefix(IMAGE_SET_LINE_DELIMITER)) {
			// Example: "# image_7d12g_Background 15000 15001 15002 15003 15004 15005"
			// This seems to only occur once in Hercules (even though there are other titles that have
			// image set actors).  For now, just print it raw but don't process it.
			debugC(5, kDebugLoading, "%s: image set: %s", __func__, line.c_str());

		} else {
			// Try to actually parse this line.
			(this->*sectionParserMethod)(line);
		}

		shouldKeepReading = !file.err() && !file.eos();
	}
}

void Profile::load() {
	const Common::Path FILENAME = "PROFILE._ST";
	Common::File file;
	if (!file.open(FILENAME)) {
		debugC(5, kDebugLoading, "%s: Could not open profile %s. Entity names will not be available.",
			__func__, FILENAME.toString().c_str());
		return;
	}
	parseVersionInfo(file.readLine());

	readSection(file, &Profile::parseContextInfo);
	debugC(5, kDebugLoading, "%s: Read %d context infos", __func__, _contexts.size());

	readSection(file, &Profile::parseAssetInfo);
	debugC(5, kDebugLoading, "%s: Read %d asset infos", __func__, _assets.size());

	readSection(file, &Profile::parseFileInfo);
	debugC(5, kDebugLoading, "%s: Read %d file infos", __func__, _files.size());

	readSection(file, &Profile::parseVariableInfo);
	debugC(5, kDebugLoading, "%s: Read %d variable infos", __func__, _variables.size());

	readSection(file, &Profile::parseParamTokenInfo);
	debugC(5, kDebugLoading, "%s: Read %d param token infos", __func__, _paramTokens.size());

	readSection(file, &Profile::parseScriptConstantInfo);
	debugC(5, kDebugLoading, "%s: Read %d script constant infos", __func__, _constants.size());

	file.close();
}

void Profile::parseVersionInfo(const Common::String &line) {
	// Example: "_Version3.3_ _MAC_"
	Common::StringTokenizer tokenizer(line);
	_versionNumber = tokenizer.nextToken();
	_platform = tokenizer.nextToken();
	debugC(5, kDebugLoading, "%s: version: %s; platform: %s", __func__,  _versionNumber.c_str(), _platform.c_str());
}

void Profile::parseContextInfo(const Common::String &line) {
	// Example: "Context Root_6c00 2929877932"
	ProfileContextInfo contextInfo;
	Common::StringTokenizer tokenizer(line);
	contextInfo.type = tokenizer.nextToken();
	contextInfo.name = tokenizer.nextToken();
	Common::String unk1Str = tokenizer.nextToken();
	contextInfo.unk1 = atol(unk1Str.c_str());
	_contexts.push_back(contextInfo);

	debugC(5, kDebugLoading, "%s: \"%s\" -> type: %s; name: %s; unk1: %d",
		__func__, line.c_str(), contextInfo.type.c_str(), contextInfo.name.c_str(), contextInfo.unk1);
}

void Profile::parseAssetInfo(const Common::String &line) {
	// Examples:
	//  - No channel: "Puppy_Transition 113 0"
	//  - One channel: "snd_6c16_NoteLevel1 1436 74"
	//  - Multiple channels: "movie_6cb1_LayeredBumpers 154 111 112 113"
	//		(Currently only stream movies seem to have multiple channels.)
	Common::StringTokenizer tokenizer(line);
	ProfileAssetInfo assetInfo;
	assetInfo.name = tokenizer.nextToken();
	Common::String idAsString = tokenizer.nextToken();
	assetInfo.id = atoi(idAsString.c_str());
	debugCN(5, kDebugLoading, "%s: \"%s\" -> name %s; id: %d; channelIdents: [ ",
		__func__, line.c_str(), assetInfo.name.c_str(), assetInfo.id);

	// The channel IDs presented here must be transformed into the actual channel IDs
	// (FourCCs) seen when reading chunks by converting them to hex and adding an offset.
	// So, for example, "snd_6c16_NoteLevel1 1436 74" has actual channel ID "a04a", since
	// 74 in hex is 0x4A, and then we add 0xA000.
	const uint CHANNEL_ID_OFFSET_TO_OBTAIN_REAL_CHUNK_FOURCC = 0xA000;
	while (!tokenizer.empty()) {
		Common::String channelIdentAsString = tokenizer.nextToken();
		uint channelIdentAsUnoffsetInt = atoi(channelIdentAsString.c_str());
		if (channelIdentAsUnoffsetInt != 0) {
			uint channelIdentAsInt = atoi(channelIdentAsString.c_str()) + CHANNEL_ID_OFFSET_TO_OBTAIN_REAL_CHUNK_FOURCC;
			assetInfo.channelIdents.push_back(channelIdentAsInt);
			_channelIdentsAsIntToAssetId.setVal(channelIdentAsInt, assetInfo.id);
			debugCN(5, kDebugLoading, "%x ", channelIdentAsInt);
		}
	}
	debugC(5, kDebugLoading, "]"); // Close out the printed channel idents list.
	_assets.setVal(assetInfo.id, assetInfo);
}

void Profile::parseFileInfo(const Common::String &line) {
	Common::StringTokenizer tokenizer(line);
	ProfileFileInfo fileInfo;
	fileInfo.name = tokenizer.nextToken();
	Common::String idStr = tokenizer.nextToken();
	fileInfo.id = atoi(idStr.c_str());
	_files.setVal(fileInfo.id, fileInfo);

	debugC(5, kDebugLoading, "%s: \"%s\" -> filename: %s fileId: %d",
		__func__, line.c_str(), fileInfo.name.c_str(), fileInfo.id);
}

void Profile::parseVariableInfo(const Common::String &line) {
	Common::StringTokenizer tokenizer(line);
	ProfileVariableInfo variableInfo;
	variableInfo.name = tokenizer.nextToken();
	Common::String idStr = tokenizer.nextToken();
	variableInfo.id = atoi(idStr.c_str());
	_variables.setVal(variableInfo.id, variableInfo);

	debugC(5, kDebugLoading, "%s: \"%s\" -> name: %s id: %d",
		__func__, line.c_str(), variableInfo.name.c_str(), variableInfo.id);
}

void Profile::parseParamTokenInfo(const Common::String &line) {
	Common::StringTokenizer tokenizer(line);
	ProfileParamTokenInfo paramTokenInfo;
	paramTokenInfo.name = tokenizer.nextToken();
	Common::String idStr = tokenizer.nextToken();
	paramTokenInfo.id = (uint16)atoi(idStr.c_str());
	_paramTokens.setVal(paramTokenInfo.id, paramTokenInfo);

	debugC(5, kDebugLoading, "%s: \"%s\" -> name: %s id: %d",
		__func__, line.c_str(), paramTokenInfo.name.c_str(), paramTokenInfo.id);
}

void Profile::parseScriptConstantInfo(const Common::String &line) {
	Common::StringTokenizer tokenizer(line);
	ProfileScriptConstantInfo constantInfo;
	constantInfo.name = tokenizer.nextToken();
	if (!tokenizer.empty()) {
		// There is no type information stored here, so we will just store the
		// textual value of the constant as is.
		constantInfo.value = tokenizer.nextToken();
	}
	_constants.push_back(constantInfo);

	debugC(5, kDebugLoading, "%s: \"%s\" -> name: %s value: %s",
		__func__, line.c_str(), constantInfo.name.c_str(), constantInfo.value.c_str());
}

Common::String Profile::formatActorName(uint actorId, bool attemptToGetType) {
	// If requested, try to get the actor type by looking up the loaded actor
	if (attemptToGetType) {
		Actor *actor = g_engine->getImtGod()->getActorById(actorId);
		if (actor != nullptr) {
			return formatActorName(actor);
		}
	}

	Common::String formattedName;
	const Common::String &actorName = _assets.getValOrDefault(actorId).name;
	if (!actorName.empty()) {
		formattedName = Common::String::format("%s (%d)", actorName.c_str(), actorId);
	} else {
		formattedName = Common::String::format("%d", actorId);
	}
	return formattedName;
}

Common::String Profile::formatActorName(const Actor *actor) {
	if (actor == nullptr) {
		return "<null>";
	}

	Common::String formattedName;
	const Common::String &actorName = _assets.getValOrDefault(actor->id()).name;
	if (!actorName.empty()) {
		formattedName = Common::String::format("%s [%s %d]", actorName.c_str(), actorTypeToStr(actor->type()), actor->id());
	} else {
		// Even if we don't have a name, try to give at least some visibility by including the type.
		formattedName = Common::String::format("%s %d", actorTypeToStr(actor->type()), actor->id());
	}
	return formattedName;
}

Common::String Profile::formatFunctionName(uint functionId) {
	// Only in PROFILE._ST, the function ID is reported with 19900 added,
	// so function 100 would be reported as 20000. But in bytecode, the
	// zero-based ID is used.
	Common::String formattedName;
	uint offsetFunctionId = functionId + 19900;
	const Common::String &functionName = _assets.getValOrDefault(offsetFunctionId).name;
	if (!functionName.empty()) {
		// Report the function ID as it appears in bytecode, so without the odd offset added.
		formattedName = Common::String::format("%s (%d)", functionName.c_str(), functionId);
	} else {
		// This might be a built-in function, in which case we can try to get the built-in name.
		formattedName = Common::String::format("%s (%d)", builtInFunctionToStr(functionId), functionId);
	}
	return formattedName;
}

Common::String Profile::formatFileName(uint fileId) {
	Common::String formattedName;
	const Common::String &fileName = _files.getValOrDefault(fileId).name;
	if (!fileName.empty()) {
		formattedName = Common::String::format("%s (%d)", fileName.c_str(), fileId);
	} else {
		formattedName = Common::String::format("%d", fileId);
	}
	return formattedName;
}

Common::String Profile::formatVariableName(uint variableId) {
	Common::String formattedName;
	const Common::String &variableName = _variables.getValOrDefault(variableId).name;
	if (!variableName.empty()) {
		formattedName = Common::String::format("%s (%d)", variableName.c_str(), variableId);
	} else {
		formattedName = Common::String::format("%d", variableId);
	}
	return formattedName;
}

Common::String Profile::formatParamTokenName(uint paramToken) {
	Common::String formattedName;
	const Common::String &paramTokenName = _paramTokens.getValOrDefault(paramToken).name;
	if (!paramTokenName.empty()) {
		formattedName = Common::String::format("%s (%d)", paramTokenName.c_str(), paramToken);
	} else {
		formattedName = Common::String::format("%d", paramToken);
	}
	return formattedName;
}

Common::String Profile::formatAssetNameForChannelIdent(uint channelIdentAsTag) {
	Common::String formattedName;
	if (channelIdentAsTag == MKTAG('i', 'g', 'o', 'd')) {
		formattedName = "ImtGod";
	} else {
		Common::String channelIdentAsString = Common::tag2string(channelIdentAsTag);
		uint channelIdentAsInt = strtol(channelIdentAsString.c_str(), nullptr, 16);
		if (_channelIdentsAsIntToAssetId.contains(channelIdentAsInt)) {
			uint assetId = _channelIdentsAsIntToAssetId.getVal(channelIdentAsInt);
			formattedName = Common::String::format("%s [%s]", channelIdentAsString.c_str(), formatActorName(assetId).c_str());
		} else {
			formattedName = Common::String::format("%s", tag2str(channelIdentAsTag));
		}
	}
	return formattedName;
}

} // End of namespace MediaStation
