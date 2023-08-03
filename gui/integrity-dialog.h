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

#ifndef GUI_INTEGRITY_DIALOG_H
#define GUI_INTEGRITY_DIALOG_H

#include "backends/networking/curl/postrequest.h"

#include "common/array.h"
#include "common/formats/json.h"
#include "common/str.h"

#include "gui/dialog.h"
#include "gui/widget.h"

namespace GUI {

enum {
	OK = 0,
	MISSING = 1,
	CHECKSUM_MISMATCH = 2,
	SIZE_MISMATCH = 3,
	UNKNOWN = 4
};

class IntegrityDialog : Dialog {
	Common::String _endpoint;
	Common::String _gamePath;
	Common::String _gameid;
	Common::String _engineid;
	Common::String _extra;
	Common::String _platform;
	Common::String _language;

	int _totalSize;
	int _calculatedSize;
	int _progressPercentage;

	bool _error;
	Common::Array<int> *_results;

	SliderWidget *_progressBar;

public:
	IntegrityDialog(Common::String endpoint, Common::String gameConfig);
	~IntegrityDialog();

	void sendJSON();

private:
	void calculateTotalSize(Common::String gamePath);

	void checksumResponseCallback(Common::JSONValue *r);
	void errorCallback(Networking::ErrorResponse error);
	Common::Array<Common::StringArray> generateChecksums(Common::String gamePath, Common::Array<Common::StringArray> &fileChecksums);
	Common::JSONValue *generateJSONRequest(Common::String gamePath, Common::String gameid, Common::String engineid, Common::String extra, Common::String platform, Common::String language);

	void parseJSON(Common::JSONValue *response);
};

} // End of namespace GUI

#endif // GUI_INTEGRITY_DIALOG_H
