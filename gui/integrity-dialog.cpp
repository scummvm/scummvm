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

#include "gui/integrity-dialog.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"

namespace GUI {

IntegrityDialog::IntegrityDialog(Common::String endpoint, Common::String gameConfig) : Dialog("GameOptions_IntegrityDialog") {
	_endpoint = endpoint;
	_gamePath = ConfMan.get("path", gameConfig);
	_gameid = ConfMan.get("gameid", gameConfig);
	_engineid = ConfMan.get("engineid", gameConfig);
	_extra = ConfMan.get("extra", gameConfig);
	_platform = ConfMan.get("platform", gameConfig);
	_language = ConfMan.get("language", gameConfig);
}

IntegrityDialog::~IntegrityDialog() {
}

Common::Array<Common::StringArray> IntegrityDialog::generateChecksums(Common::String gamePath, Common::Array<Common::StringArray> &fileChecksums) {
	const Common::FSNode dir(gamePath);

	if (!dir.exists() || !dir.isDirectory())
		return {};

	Common::FSList fileList;
	if (!dir.getChildren(fileList, Common::FSNode::kListAll))
		return {};

	if (fileList.empty())
		return {};

	// Process the files and subdirectories in the current directory recursively
	for (Common::FSList::const_iterator it = fileList.begin(); it != fileList.end(); it++) {
		const Common::FSNode &entry = *it;

		if (entry.isDirectory())
			generateChecksums(entry.getPath(), fileChecksums);
		else {
			const Common::Path filename(entry.getPath());
			Common::File file;
			if (!file.open(entry))
				continue;

			Common::Array<Common::String> fileChecksum = {filename.toString()};
			// Various checksizes
			for (auto size : {0, 5000, 1024 * 1024}) {
				fileChecksum.push_back(Common::computeStreamMD5AsString(file, size).c_str());
				file.seek(0);
			}
			// Tail checksums with checksize 5000
			file.seek(-5000, SEEK_END);
			fileChecksum.push_back(Common::computeStreamMD5AsString(file).c_str());

			file.close();
			fileChecksums.push_back(fileChecksum);
		}
	}

	return fileChecksums;
}

Common::JSONValue *IntegrityDialog::generateJSONRequest(Common::String gamePath, Common::String gameid, Common::String engineid, Common::String extra, Common::String platform, Common::String language) {
	Common::Array<Common::StringArray> fileChecksums = {};
	fileChecksums = generateChecksums(gamePath, fileChecksums);
	Common::JSONObject requestObject;

	requestObject.setVal("gameid", new Common::JSONValue(gameid));
	requestObject.setVal("engineid", new Common::JSONValue(engineid));
	requestObject.setVal("extra", new Common::JSONValue(extra));
	requestObject.setVal("platform", new Common::JSONValue(platform));
	requestObject.setVal("language", new Common::JSONValue(language));

	Common::JSONArray filesObject;

	for (Common::StringArray fileChecksum : fileChecksums) {
		Common::JSONObject file;
		Common::String relativePath = fileChecksum[0].substr(gamePath.size() + 1); // +1 to remove the first '/'
		file.setVal("name", new Common::JSONValue(relativePath));

		auto tempNode = Common::FSNode(fileChecksum[0]);
		Common::File tempFile;
		if (!tempFile.open(tempNode))
			continue;
		long long fileSize = tempFile.size();
		tempFile.close();

		file.setVal("size", new Common::JSONValue(fileSize));

		Common::JSONArray checksums;
		Common::StringArray checkcodes = {"md5", "md5-5000", "md5-1M", "md5-t-5000"};

		int index = -1;
		for (Common::String val : fileChecksum) {
			index++;

			Common::JSONObject checksum;
			if (index < 1) {
				continue;
			}

			checksum.setVal("type", new Common::JSONValue(checkcodes[index - 1]));
			checksum.setVal("checksum", new Common::JSONValue(val));

			checksums.push_back(new Common::JSONValue(checksum));
		}
		file.setVal("checksums", new Common::JSONValue(checksums));

		filesObject.push_back(new Common::JSONValue(file));
	}

	requestObject.setVal("files", new Common::JSONValue(filesObject));

	Common::JSONValue *request = new Common::JSONValue(requestObject);
	return request;
}

void IntegrityDialog::checksumResponseCallback(Common::JSONValue *r) {
	debug(r->stringify().c_str());
}

void IntegrityDialog::errorCallback(Networking::ErrorResponse error) {
	debug("ERROR %ld: %s", error.httpResponseCode, error.response.c_str());
}

void IntegrityDialog::sendJSON() {
	auto conn = new Networking::PostRequest(_endpoint,
											new Common::Callback<IntegrityDialog, Common::JSONValue *>(this, &IntegrityDialog::checksumResponseCallback),
											new Common::Callback<IntegrityDialog, Networking::ErrorResponse>(this, &IntegrityDialog::errorCallback));

	Common::JSONValue *json = generateJSONRequest(_gamePath, _gameid, _engineid, _extra, _platform, _language);
	conn->setJSONData(json);
	conn->setContentType("application/json");
	conn->start();
	delete json;
}

} // End of namespace GUI
