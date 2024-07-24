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
#include "common/macresman.h"
#include "common/md5.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "gui/gui-manager.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kResponseCmd = 'IDRC',
	kCopyEmailCmd = 'IDCE',
	kCleanupCmd = 'IDCl',
	kDownloadProgressCmd = 'DLPg',
};

struct ResultFormat {
	bool error;
	Common::U32StringArray messageText;
	Common::String emailLink;
	Common::String errorText;

	ResultFormat() {
		error = 0;
		messageText = Common::U32StringArray();
		emailLink = "";
		errorText = "";
	}

} static *g_result;

struct ChecksumDialogState {
	IntegrityDialog *dialog;
	ProcessState state;

	int totalSize;
	int calculatedSize;
	uint32 lastUpdate;

	Common::String endpoint;
	Common::Path gamePath;
	Common::String gameid;
	Common::String engineid;
	Common::String extra;
	Common::String platform;
	Common::String language;

	ChecksumDialogState() {
		state = kChecksumStateNone;
		totalSize = calculatedSize = 0;
		lastUpdate = 0;
		dialog = nullptr;
	}
} static *g_checksum_state;

uint32 getCalculationProgress() {
	if (!g_checksum_state || g_checksum_state->totalSize == 0)
		return 0;

	uint32 progress = (uint32)(100 * ((double)g_checksum_state->calculatedSize / (double)g_checksum_state->totalSize));

	return progress;
}

IntegrityDialog::IntegrityDialog(Common::String endpoint, Common::String domain) : Dialog("GameOptions_IntegrityDialog"), CommandSender(this), _close(false) {

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	Common::U32String warningMessage = _(
		"Verifying file integrity may take a long time to complete. Please wait...\n");
	_warningText = new StaticTextWidget(this, "GameOptions_IntegrityDialog.WarningText", warningMessage);
	_resultsText = new ListWidget(this, "GameOptions_IntegrityDialog.ResultsText");
	_resultsText->setNumberingMode(kListNumberingOff);
	_resultsText->setList({Common::U32String()});
	_resultsText->setVisible(false);

	_statusText = new StaticTextWidget(this, "GameOptions_IntegrityDialog.StatusText", Common::U32String::format(_("Calculating file checksums...")));
	_errorText = new StaticTextWidget(this, "GameOptions_IntegrityDialog.ErrorText", Common::U32String(""));

	uint32 progress = getCalculationProgress();
	_progressBar = new SliderWidget(this, "GameOptions_IntegrityDialog.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(progress);
	_progressBar->setEnabled(false);
	_percentLabel = new StaticTextWidget(this, "GameOptions_IntegrityDialog.PercentText", Common::String::format("%u %%", progress));
	_calcSizeLabel = new StaticTextWidget(this, "GameOptions_IntegrityDialog.DownloadSize", Common::U32String());
	_cancelButton = new ButtonWidget(this, "GameOptions_IntegrityDialog.MainButton", _("Cancel"), Common::U32String(), kCleanupCmd);

	_copyEmailButton = new ButtonWidget(this, "GameOptions_IntegrityDialog.CopyButton", _("Launch Email Client"), Common::U32String(), kCopyEmailCmd);
	_copyEmailButton->setEnabled(false);

	if (!g_checksum_state) {
		g_checksum_state = new ChecksumDialogState();
		g_checksum_state->dialog = this;

		setState(kChecksumStateCalculating);
		refreshWidgets();

		g_checksum_state->endpoint = endpoint;
		g_checksum_state->gamePath = Common::Path(ConfMan.getPath("path", domain));
		g_checksum_state->gameid = ConfMan.get("gameid", domain);
		g_checksum_state->engineid = ConfMan.get("engineid", domain);
		g_checksum_state->extra = ConfMan.get("extra", domain);
		g_checksum_state->platform = ConfMan.get("platform", domain);
		g_checksum_state->language = ConfMan.get("language", domain);
		calculateTotalSize(g_checksum_state->gamePath);
	} else {
		g_checksum_state->dialog = this;

		setState(g_checksum_state->state);
		refreshWidgets();
	}
}

IntegrityDialog::~IntegrityDialog() {
}

void IntegrityDialog::open() {
	Dialog::open();
	reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void IntegrityDialog::close() {
	if (g_checksum_state)
		g_checksum_state->dialog = nullptr;

	Dialog::close();
}

void IntegrityDialog::setState(ProcessState state) {
	g_checksum_state->state = state;

	switch (state) {
	case kChecksumStateNone:
	case kChecksumStateCalculating:
		_statusText->setLabel(Common::U32String::format(_("Calculating file checksums...")));
		_cancelButton->setLabel(_("Cancel"));
		_cancelButton->setCmd(kCleanupCmd);
		break;

	case kChecksumComplete:
		_statusText->setLabel(Common::U32String::format(_("Calculation complete")));
		_cancelButton->setVisible(true);
		_cancelButton->setLabel(_("OK"));
		_cancelButton->setCmd(kCleanupCmd);

		// Hide all elements
		_warningText->setVisible(false);
		_statusText->setVisible(false);
		_errorText->setVisible(false);
		_percentLabel->setVisible(false);
		_calcSizeLabel->setVisible(false);
		_progressBar->setVisible(false);
		break;

	case kResponseReceived:
		if (g_result->messageText.size() != 0) {
			_resultsText->setList(g_result->messageText);
		} else
			_resultsText->setList(Common::U32StringArray({g_result->errorText}));

		if (g_result->error != 0) {
			_copyEmailButton->setEnabled(true);
			_copyEmailButton->setCmd(kCopyEmailCmd);
		}

		_resultsText->setVisible(true);

		break;
	}
}

void IntegrityDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kResponseReceived:
		setState(kResponseReceived);
		break;
	case kCleanupCmd: {
		delete g_checksum_state;
		g_checksum_state = nullptr;

		delete g_result;
		g_result = nullptr;

		close();
		break;
	}
	case kDownloadProgressCmd:
		if (!_close) {
			refreshWidgets();
			g_gui.redrawFull();
		}
		break;
	case kCopyEmailCmd: {
		g_system->openUrl(g_result->emailLink);
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void IntegrityDialog::handleTickle() {
	if (_close) {
		close();
		_close = false;
		return;
	}

	if (g_checksum_state->state == kChecksumStateCalculating)
		sendJSON();

	int32 progress = getCalculationProgress();
	if (_progressBar->getValue() != progress) {
		refreshWidgets();
		g_gui.scheduleTopDialogRedraw();
	}

	Dialog::handleTickle();
}

void IntegrityDialog::reflowLayout() {
	Dialog::reflowLayout();
	refreshWidgets();
}

Common::U32String IntegrityDialog::getSizeLabelText() {
	const char *calculatedUnits, *totalUnits;
	Common::String calculated = Common::getHumanReadableBytes(g_checksum_state->calculatedSize, calculatedUnits);
	Common::String total = Common::getHumanReadableBytes(g_checksum_state->totalSize, totalUnits);
	return Common::U32String::format(_("Calculated %s %S / %s %S"), calculated.c_str(), _(calculatedUnits).c_str(), total.c_str(), _(totalUnits).c_str());
}

void IntegrityDialog::refreshWidgets() {
	uint32 progress = getCalculationProgress();
	_percentLabel->setLabel(Common::String::format("%u %%", progress));
	_calcSizeLabel->setLabel(getSizeLabelText());
	_progressBar->setValue(progress);
}

void IntegrityDialog::setError(Common::U32String &msg) {
	_errorText->setLabel(msg);

	_cancelButton->setLabel(_("Close"));
	_cancelButton->setCmd(kCleanupCmd);
}

void IntegrityDialog::calculateTotalSize(Common::Path gamePath) {
	const Common::FSNode dir(gamePath);

	if (!dir.exists() || !dir.isDirectory())
		return;

	Common::FSList fileList;
	if (!dir.getChildren(fileList, Common::FSNode::kListAll))
		return;

	if (fileList.empty())
		return;

	// Process the files and subdirectories in the current directory recursively
	for (Common::FSList::const_iterator it = fileList.begin(); it != fileList.end(); it++) {
		const Common::FSNode &entry = *it;

		if (entry.isDirectory())
			calculateTotalSize(entry.getPath());
		else {
			Common::File file;
			if (!file.open(entry))
				continue;

			g_checksum_state->totalSize += file.size();
		}
	}
}

Common::Array<Common::StringArray> IntegrityDialog::generateChecksums(Common::Path gamePath, Common::Array<Common::StringArray> &fileChecksums) {
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
			auto macFile = Common::MacResManager();
			if (macFile.open(filename)) {
				auto fileStream = macFile.openFileOrDataFork(filename);
				Common::SeekableReadStream *dataForkStream;
				if (macFile.isMacBinary(*fileStream))
					dataForkStream = macFile.openDataForkFromMacBinary(fileStream);
				else
					dataForkStream = fileStream;

				Common::Array<Common::String> fileChecksum = {filename.toString()};

				// Data fork
				// Various checksizes
				for (auto size : {0, 5000, 1024 * 1024}) {
					fileChecksum.push_back(Common::computeStreamMD5AsString(*dataForkStream, size).c_str());
					dataForkStream->seek(0);
				}
				// Tail checksums with checksize 5000
				dataForkStream->seek(-5000, SEEK_END);
				fileChecksum.push_back(Common::computeStreamMD5AsString(*dataForkStream).c_str());

				// Resource fork
				if (macFile.hasResFork()) {
					// Various checksizes
					for (auto size : {0, 5000, 1024 * 1024}) {
						fileChecksum.push_back(macFile.computeResForkMD5AsString(size).c_str());
					}
					// Tail checksums with checksize 5000
					fileChecksum.push_back(macFile.computeResForkMD5AsString(5000, true).c_str());
					fileChecksums.push_back(fileChecksum);
				}

				g_checksum_state->calculatedSize += fileStream->size();
			}

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

			g_checksum_state->calculatedSize += file.size();

			if (g_system->getMillis() > g_checksum_state->lastUpdate + 500) {
				g_checksum_state->lastUpdate = g_system->getMillis();
				sendCommand(kDownloadProgressCmd, 0);
			}

			file.close();
			fileChecksums.push_back(fileChecksum);
		}
	}

	setState(kChecksumComplete);
	return fileChecksums;
}

Common::JSONValue *IntegrityDialog::generateJSONRequest(Common::Path gamePath, Common::String gameid, Common::String engineid, Common::String extra, Common::String platform, Common::String language) {
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
		Common::Path relativePath = Common::Path(fileChecksum[0]).relativeTo(gamePath);
		file.setVal("name", new Common::JSONValue(relativePath.toConfig()));

		auto tempNode = Common::FSNode(Common::Path(fileChecksum[0]));
		Common::File tempFile;
		if (!tempFile.open(tempNode))
			continue;
		uint64 fileSize = tempFile.size();
		tempFile.close();

		file.setVal("size", new Common::JSONValue((long long)fileSize));

		Common::JSONArray checksums;
		Common::StringArray checkcodes;
		if (fileChecksum.size() == 8)
			checkcodes = {"md5-d", "md5-d-5000", "md5-d-1M", "md5-dt-5000", "md5-r", "md5-r-5000", "md5-r-1M", "md5-rt-5000"};
		else
			checkcodes = {"md5", "md5-5000", "md5-1M", "md5-t-5000"};

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

void IntegrityDialog::checksumResponseCallback(const Common::JSONValue *r) {
	debug(3, "JSON Response: %s", r->stringify().c_str());
	IntegrityDialog::parseJSON(r);

	if (g_checksum_state->dialog)
		g_checksum_state->dialog->sendCommand(kResponseReceived, 0);
}

void IntegrityDialog::errorCallback(const Networking::ErrorResponse &error) {
	warning("ERROR %ld: %s", error.httpResponseCode, error.response.c_str());
	g_result->errorText = Common::String::format("ERROR %ld: %s", error.httpResponseCode, error.response.c_str());

	if (g_checksum_state->dialog)
		g_checksum_state->dialog->sendCommand(kResponseCmd, 0);
}

void IntegrityDialog::sendJSON() {
	g_result = new ResultFormat();

	auto conn = new Networking::PostRequest(g_checksum_state->endpoint,
		new Common::Callback<IntegrityDialog, const Common::JSONValue *>(this, &IntegrityDialog::checksumResponseCallback),
		new Common::Callback<IntegrityDialog, const Networking::ErrorResponse &>(this, &IntegrityDialog::errorCallback));

	Common::JSONValue *json = generateJSONRequest(
		g_checksum_state->gamePath, g_checksum_state->gameid, g_checksum_state->engineid, g_checksum_state->extra, g_checksum_state->platform, g_checksum_state->language);
	conn->setJSONData(json);
	conn->setContentType("application/json");
	conn->start();
	delete json;
}

void IntegrityDialog::parseJSON(const Common::JSONValue *response) {
	Common::JSONObject responseObject = response->asObject();
	int responseError = responseObject.getVal("error")->asIntegerNumber();

	Common::U32StringArray messageText;
	if (responseError == -1) { // Unknown variant
		g_result->error = true;

		Common::String fileset = responseObject.getVal("fileset")->asString();

		Common::String emailSubj = "Unknown game variant fileset ";
		Common::String emailBody = "Fileset %s is a new or unknown fileset, the game details are:\n"
				"    gameid: %s\n"
				"    platform: %s\n"
				"    language: %s\n"
				" \n"
				"Below please describe the details of your release:\n";

		Common::String emailText =
			Common::String::format(emailBody.c_str(),
					   fileset.c_str(), g_checksum_state->gameid.c_str(), g_checksum_state->platform.c_str(),
					   g_checksum_state->language.c_str());

		Common::String emailLink = Common::String::format("mailto:integrity@scummvm.org?subject=%s&body=%s",
				Common::percentEncodeString(Common::String("Subject: ") + emailSubj + fileset).c_str(),
				Common::percentEncodeString(emailText).c_str());

		Common::U32String message = _(
				"### Results\n"
				"Your set of game files seems to be unknown to us.\n"
				"\n"
				"If you are sure that this is a valid unknown variant, "
				"please send the following e-mail to integrity@scummvm.org");

		Common::U32StringTokenizer mtok(message, "\n");
		for (auto &line : mtok.split())
			messageText.push_back(line);

		messageText.push_back(Common::U32String(""));
		messageText.push_back(Common::U32String("To: integrity@scummvm.org"));
		messageText.push_back(Common::U32String::format("Subject: %s%s", emailSubj.c_str(), fileset.c_str()));
		messageText.push_back(Common::U32String(""));

		Common::StringTokenizer tok(emailText, "\n");
		for (auto &line : tok.split())
			messageText.push_back(line);

		g_result->messageText = messageText;
		g_result->emailLink = emailLink;
		return;

	} else if (responseError == 2) { // Fileset is empty
		messageText.push_back(_("The game doesn't seem to have any files. Are you sure the path is correct?"));

		g_result->messageText = messageText;
		return;
	} else if (responseError == 3) { // Game does not have any metadata
		messageText.push_back(_("The game doesn't seem to have any metadata associated with it, so it is unable to be matched. Please fill in the correct metadata for the game."));

		g_result->messageText = messageText;
		return;
	}

	Common::Array<int> results = Common::Array<int>(5, 0);
	for (Common::JSONValue *fileJSON : responseObject.getVal("files")->asArray()) {
		Common::String name = fileJSON->asObject().getVal("name")->asString();
		Common::String status = fileJSON->asObject().getVal("status")->asString();

		if (status == "ok") {
			results[OK]++;
			continue;
		} else if (status == "missing")
			results[MISSING]++;
		else if (status == "checksum_mismatch")
			results[CHECKSUM_MISMATCH]++;
		else if (status == "size_mismatch")
			results[SIZE_MISMATCH]++;
		else if (status == "unknown_file")
			results[UNKNOWN]++;

		messageText.push_back(Common::String::format("%s %s\n", name.c_str(), status.c_str()));
	}

	if (messageText.size() == 0)
		messageText.push_back(_("Files all OK"));
	else {
		g_result->error = true;

		Common::String resultSummary = "\n\nTotal: ";
		resultSummary += Common::U32String::format("%d OK, %d missing, %d mismatch, %d unknown files",
												   results[OK], results[MISSING], results[SIZE_MISMATCH] + results[CHECKSUM_MISMATCH], results[UNKNOWN]);

		messageText.push_back(resultSummary);
	}

	g_result->messageText = messageText;
}

} // End of namespace GUI
